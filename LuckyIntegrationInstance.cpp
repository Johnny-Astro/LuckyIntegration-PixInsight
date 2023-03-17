#include <pcl/AutoViewLock.h>
#include <pcl/Console.h>
#include <pcl/File.h>
#include <pcl/FileFormat.h>
#include <pcl/FileFormatInstance.h>
#include <pcl/MuteStatus.h>
#include <pcl/Mutex.h>
#include <pcl/MultiscaleMedianTransform.h>
#include <pcl/ProcessInterface.h>
#include <pcl/StandardStatus.h>
#include <pcl/View.h>
#include <pcl/XML.h>

#include "LuckyIntegrationInstance.h"
#include "LuckyIntegrationParameters.h"

namespace pcl
{

static bool LoadImage(Image& image, const String& filePath)
{
    if (!File::Exists(filePath))
        throw Error(filePath + ": not found.");

    FileFormat format(File::ExtractExtension(filePath), true/*toRead*/, false/*toWrite*/);
    FileFormatInstance file(format);

    ImageDescriptionArray images;

    if (!file.Open(images, filePath))
        throw CaughtException();
    if (images.IsEmpty())
        throw Error(filePath + ": Empty image file.");
    if (!file.SelectImage(0))
        throw CaughtException();
    if (!file.ReadImage(image))
        throw CaughtException();
    if (!file.Close())
        throw CaughtException();

    return true;
}

struct ImageThreadGlobalData
{
    StringList inputFilenames;
    int width;
    int height;
    AtomicInt imageIdx;
    Mutex lock;
};

class ImageThread : public Thread
{
private:
    String m_threadErrorMsg;

protected:
    int m_id;
    static ImageThreadGlobalData m_globalData;
    LuckyIntegrationInstance* m_instance;

    ImageThread(int id, LuckyIntegrationInstance* instance)
        : m_id(id)
        , m_instance(instance)
        , m_threadErrorMsg("")
    {
        if (m_id != 0)
            return;

        m_globalData.inputFilenames.Clear();
        m_globalData.width = 0;
        m_globalData.height = 0;
        m_globalData.imageIdx = 0;

        File::Find find;
        FindFileInfo info;
        find.Begin(m_instance->p_inputPath + "\\*.fit");
        while (find.NextItem(info))
        {
            m_globalData.inputFilenames.Add(m_instance->p_inputPath + "\\" + info.name);
        }
        find.End();
        find.Begin(m_instance->p_inputPath + "\\*.fits");
        while (find.NextItem(info))
        {
            m_globalData.inputFilenames.Add(m_instance->p_inputPath + "\\" + info.name);
        }
        find.End();
        if (m_globalData.inputFilenames.Length() == 0)
            throw Error("No *.fit / *.fits files in the selected directory.");
        m_globalData.inputFilenames.Sort();
    }

    virtual ~ImageThread()
    {
    }

    void Run() override
    {
        try {
            while (1)
            {
                Console console;
                if (console.AbortRequested())
                    throw ProcessAborted();
                NativeImage srcImage;
                int imageIdx;
                if (!read(srcImage, imageIdx))
                    break;
                NativeImage dstImage;
                process(dstImage, srcImage, imageIdx);
             }
        }
        catch (...) {
            try {
                throw;
            }
            catch (ProcessAborted&)
            {
                m_threadErrorMsg = "User aborted";
                throw;
            }
            catch (Exception& x) {
                m_threadErrorMsg = x.Message();
            }
            catch (std::bad_alloc&) {
                m_threadErrorMsg = "Out of memory";
            }
            catch (...) {
                m_threadErrorMsg = "Unknown error";
            }
        }
    }

    virtual bool read(NativeImage& srcImage, int& imageIdx)
    {
        String err = "";
        bool done = false;

        m_globalData.lock.Lock();
        imageIdx = m_globalData.imageIdx.Load();
        if (imageIdx >= int(m_globalData.inputFilenames.Length() * (m_instance->p_framePercentage * 0.01)))
            done = true;
        else if ((m_instance->p_routine == LIRoutine::StarDetectionPreview) && (imageIdx >= 1))
            done = true;
        if (!done)
            m_globalData.imageIdx.Increment();
        m_globalData.lock.Unlock();

        if (done)
            return false;

        Image image;
        LoadImage(image, m_globalData.inputFilenames[imageIdx]);
        m_globalData.lock.Lock();
        if (m_globalData.width == 0)
        {
            m_globalData.width = image.Width();
            m_globalData.height = image.Height();
        }
        m_globalData.lock.Unlock();
        if ((m_globalData.width != image.Width()) || (m_globalData.height != image.Height()))
        {
            throw Error("Image dimension mismatches.");
        }
        srcImage.allocate<float>(image.Width(), image.Height());
        srcImage.copyRaw(image.PixelData());

        return true;
    }

    virtual void process(NativeImage& dstImage, const NativeImage& srcImage, int imageIdx) = 0;

public:
    template<class T>
    static void dispatch(LuckyIntegrationInstance* instance, int numThreads = 0)
    {
        int n = (numThreads > 0) ? numThreads : Thread::NumberOfThreads(PCL_MAX_PROCESSORS, 1);
        Console().WriteLn(String().Format("Using %d worker threads...", n));
        ReferenceArray<T> threads;
        for (int i = 0; i < n; i++)
        {
            T* thread = new T(i, instance);
            thread->Start(ThreadPriority::DefaultMax, i);
            threads << thread;
        }
        while (1)
        {
            ProcessInterface::ProcessEvents();
            pcl::Sleep(100);
            Console().Write(String().Format("<clreol>%d / %d source images processed.", m_globalData.imageIdx.Load(), m_globalData.inputFilenames.Length()) + "<bol>");
            bool completed = true;
            for (T& t : threads)
            {
                // t.FlushConsoleOutputText();
                if (t.IsActive())
                    completed = false;
            }
            if (completed)
                break;
        }
        Console().WriteLn("Done.<clreol>");
        String err = "";
        for (T& t : threads)
            if (t.m_threadErrorMsg != "")
            {
                err = t.m_threadErrorMsg;
                break;
            }
        threads.Destroy();
        if (err != "")
            throw Error(err);
    }
};

ImageThreadGlobalData ImageThread::m_globalData;

class StarDetectionThread : public ImageThread
{
    void cosmeticCorrection(NativeImage& dstImg, const NativeImage& srcImg, bool invalidate)
    {
        const int half_box_size = 1;
        int w = srcImg.width();
        int h = srcImg.height();
        dstImg.allocate<float>(w, h);
        // Calculate 3x3 mean
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                // Calculate mean
                float sum = 0.0f;
                int n = 0;
                for (int dy = y - half_box_size; dy <= y + half_box_size; dy++)
                {
                    if ((dy < 0) || (dy >= h))
                        continue;
                    for (int dx = x - half_box_size; dx <= x + half_box_size; dx++)
                    {
                        if ((dx < 0) || (dx >= w))
                            continue;
                        sum += srcImg.get(dx, dy);
                        n++;
                    }
                }
                float mean = sum / n;
                // Calculate variance
                sum = 0.0f;
                n = 0;
                for (int dy = y - half_box_size; dy <= y + half_box_size; dy++)
                {
                    if ((dy < 0) || (dy >= h))
                        continue;
                    for (int dx = x - half_box_size; dx <= x + half_box_size; dx++)
                    {
                        if ((dx < 0) || (dx >= w))
                            continue;
                        float d = srcImg.get(dx, dy) - mean;
                        sum += d * d;
                        n++;
                    }
                }
                float var = sum / n;
                // Correction
                float v = srcImg.get(x, y);
                float d = v - mean;
                if (d * d > 4.0f * var)    // 2 sigma
                {
                    if (invalidate)
                    {
                        dstImg.set(nanf(""), x, y);
                    }
                    else
                    {
                        Array<float> vals;
                        int n = 0;
                        for (int dy = y - half_box_size; dy <= y + half_box_size; dy++)
                        {
                            if ((dy < 0) || (dy >= h))
                                continue;
                            for (int dx = x - half_box_size; dx <= x + half_box_size; dx++)
                            {
                                if ((dx < 0) || (dx >= w))
                                    continue;
                                vals.Append(srcImg.get(dx, dy));
                                n++;
                            }
                        }
                        vals.Sort();
                        dstImg.set(vals[n / 2], x, y);
                    }
                }
                else
                {
                    dstImg.set(v, x, y);
                }
            }
        }
    }

    void getBackground(NativeImage& dstImg, const NativeImage& srcImg)
    {
        // Extract background
        ImageVariant medImg;
        medImg.CreateFloatImage();
        medImg.AllocateData(srcImg.width(), srcImg.height());
        CopyMemory(static_cast<Image&>(*medImg).PixelData(), srcImg.data(), srcImg.size());
        medImg.SetStatusCallback(nullptr);
        MultiscaleMedianTransform mmt(6);
        mmt << medImg;
        mmt.DisableLayer(0);
        mmt.DisableLayer(1);
        mmt.DisableLayer(2);
        mmt.DisableLayer(3);
        mmt.DisableLayer(4);
        mmt.DisableLayer(5);
        mmt >> medImg;
        medImg.Truncate(0.0f, 1.0f);
        dstImg.allocate<float>(srcImg.width(), srcImg.height());
        CopyMemory(dstImg.data(), static_cast<Image&>(*medImg).PixelData(), dstImg.size());
    }

    float calcFwhm(const Array<float>& v)
    {
        int b = int(v.Length() / 2);
        float a = v[b];
        float best_c = 0.0f, min_e = 1.0e+6;
        for (float c = 0.1f; c < 20.0f; c += 0.1f)
        {
            float e = 0.0f;
            for (int x = 0; x <= 2 * b; x++)
            {
                float g = a * Exp(-(x - b) * (x - b) / (2 * c * c));
                g -= v[x];
                e += g * g;
            }
            if (e < min_e)
            {
                min_e = e;
                best_c = c;
            }
        }
        return best_c * 2.35482f;
    }

    void starDetection(Array<Star>& stars, NativeImage& dstImg, const NativeImage& srcImg)
    {
        NativeImage tmpImg;
        int w = srcImg.width();
        int h = srcImg.height();
        tmpImg.allocate<float>(w, h);
        int half_box_size = int(m_instance->p_approxFwhm + 0.5);
        int step = (half_box_size * 2 + 1) / 7;
        if (step < 1)
            step = 1;

        // Calculate box mean
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float sum = 0.0f;
                int n = 0;
                for (int dy = y - half_box_size; dy <= y + half_box_size; dy += step)
                {
                    if ((dy < 0) || (dy >= h))
                        continue;
                    for (int dx = x - half_box_size; dx <= x + half_box_size; dx += step)
                    {
                        if ((dx < 0) || (dx >= w))
                            continue;
                        float v = srcImg.get(dx, dy);
                        if (isnan(v))
                            continue;
                        sum += v;
                        n++;
                    }
                }
                tmpImg.set(sum / n, x, y);
            }
        }

        // Substract
        tmpImg.rsub(srcImg);

        // Binarize + 5x5 median
        NativeImage binImg;
        binImg.allocate<float>(w, h);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
            {
                int n = 0;
                for (int dy = y - 2; dy <= y + 2; dy++)
                {
                    if ((dy < 0) || (dy >= h))
                        continue;
                    for (int dx = x - 2; dx <= x + 2; dx++)
                    {
                        if ((dx < 0) || (dx >= w))
                            continue;
                        float v = tmpImg.get(dx, dy);
                        if (isnan(v))
                            continue;
                        if (v >= m_instance->p_minPeak)
                            n++;
                    }
                }
                if (n >= 5)
                    binImg.set(1.0f, x, y);
                else
                    binImg.set(0.0f, x, y);
            }

        // Get connected components
        Array<Star> detections;
        detections.Clear();
        Array<Array<Point>> components;
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
            {
                Array<Point> points;
                Array<Point> stack;
                if (binImg.get(x, y) == 0.0f)
                    continue;
                points.Append(Point(x, y));
                binImg.set(0.0f, x, y);
                stack.Append(Point(x, y));
                while (stack.Length() > 0)
                {
                    Point p = *(stack.ReverseBegin());
                    stack.Remove(stack.ReverseBegin());
                    const int neighbor_x[] = {-1, 0, 1, -1, 1, -1, 0, 1};
                    const int neighbor_y[] = {-1, -1, -1, 0, 0, 1, 1, 1};
                    for (int i = 0; i < 8; i++)
                    {
                        int dx = p.x + neighbor_x[i];
                        int dy = p.y + neighbor_y[i];
                        if ((dx < 0) || (dx >= w) || (dy < 0) || (dy >= h))
                            continue;
                        if (binImg.get(dx, dy) > 0.0f)
                        {
                            points.Append(Point(dx, dy));
                            binImg.set(0.0f, dx, dy);
                            stack.Append(Point(dx, dy));
                        }
                    }
                }
                components.Append(points);
            }

        // Components -> stars
        int n_stars = 0;
        for (const auto& points : components)
        {
            Star s{};
            for (int i = 0; i < points.Length(); i++)
            {
                s.x += points[i].x;
                s.y += points[i].y;
            }
            s.x = s.x / points.Length() + 0.5f;
            s.y = s.y / points.Length() + 0.5f;
            int range = m_instance->p_approxFwhm * 2.0f + 0.5f;
            if ((s.x < range) || (s.x >= w - range) || (s.y < range) || (s.y >= h - range))
                continue;
            // Calculate mass center and peak
            float peak = 0.0f;
            float mass = 0.0f;
            float center_x = 0.0f, center_y = 0.0f;
            for (int y = s.y - range; y <= s.y + range; y++)
                for (int x = s.x - range; x <= s.x + range; x++)
                {
                    float v = srcImg.get(x, y);
                    if (isnan(v))
                        continue;
                    if (v > peak)
                        peak = v;
                    v -= m_instance->m_backgroundImage.getBilinear(s.x, s.y);;
                    mass += v;
                    center_x += x * v;
                    center_y += y * v;
                }
            s.x = center_x / mass;
            s.y = center_y / mass;
            s.background = m_instance->m_backgroundImage.getBilinear(s.x, s.y);
            s.peak = peak;
            if ((s.x < range) || (s.x >= w - range) || (s.y < range) || (s.y >= h - range))
                continue;
            // Calculate size
            Array<float> x_values(range * 2 + 1), y_values(range * 2 + 1);
            for (int i = -range; i <= range; i++)
            {
                x_values[i + range] = srcImg.getBilinear(s.x + i, s.y) - s.background;
                y_values[i + range] = srcImg.getBilinear(s.x, s.y + i) - s.background;
            }
            s.sizeX = calcFwhm(x_values);
            s.sizeY = calcFwhm(y_values);
            s.id = n_stars++;
            detections.Append(s);
        }

        // Filtering
        for (auto& s : detections)
        {
            if (s.peak == 0.0f)
                continue;
            // peak
            if (s.peak < m_instance->p_minPeak)
                s.peak = 0.0f;
            // saturation
            if (s.peak > m_instance->p_saturationThreshold)
                s.peak = 0.0f;
            // size
            if (s.sizeX < m_instance->p_approxFwhm * 0.5f)
                s.peak = 0.0f;
            if (s.sizeY < m_instance->p_approxFwhm * 0.5f)
                s.peak = 0.0f;
            // distance
            for (const auto& s1 : detections)
            {
                if (s.id == s1.id)
                    continue;
                if ((s.x - s1.x) * (s.x - s1.x) + (s.y - s1.y) * (s.y - s1.y) < m_instance->p_approxFwhm * m_instance->p_approxFwhm * 16.0f)
                {
                    s.peak = 0.0f;
                    break;
                }
            }
        }

        // Output
        stars.Clear();
        n_stars = 0;
        for (auto& s : detections)
        {
            if (s.peak > 0.0f)
            {
                s.id = n_stars++;
                stars.Append(s);
            }
        }

        if (m_instance->p_routine != LIRoutine::StarDetectionPreview)
            return;

        dstImg.allocate<float>(w, h);
        dstImg.copy(srcImg);
        for (const auto& s : stars)
        {
            float hx_2 = (s.sizeX - 1.0f) * 0.5f;
            float hy_2 = (s.sizeY - 1.0f) * 0.5f;
            for (int x = s.x - hx_2 + 0.5f; x <= s.x + hx_2 + 0.5f; x++)
            {
                dstImg.set(1.0f, x, s.y - hy_2 + 0.5f);
                dstImg.set(1.0f, x, s.y + hy_2 + 0.5f);
            }
            for (int y = s.y - hy_2 + 0.5f; y <= s.y + hy_2 + 0.5f; y++)
            {
                dstImg.set(1.0f, s.x - hx_2 + 0.5f, y);
                dstImg.set(1.0f, s.x + hx_2 + 0.5f, y);
            }
        }
    }

    void starMovement(Array<Star>& stars, NativeImage& dstImg, const Array<Star>& prevStars, const NativeImage& srcImg)
    {
        int w = srcImg.width();
        int h = srcImg.height();
        m_globalData.lock.Lock();
        if (!dstImg.isAllocated())
        {
            dstImg.allocate<float>(w, h);
            dstImg.zero();
        }
        m_globalData.lock.Unlock();
        int range = m_instance->p_approxFwhm * 2.0f + 0.5f;
        for (const auto& s : prevStars)
        {
            Star star = s;
            // Recalculate mass center
            if ((s.x < range) || (s.x >= w - range) || (s.y < range) || (s.y >= h - range))
            {
                star.peak = 0.0f;
                stars.Append(star);
                continue;
            }
            // Calculate mass center and peak
            float peak = 0.0f;
            float mass = 0.0f;
            float center_x = 0.0f, center_y = 0.0f;
            for (int y = s.y - range; y <= s.y + range; y++)
                for (int x = s.x - range; x <= s.x + range; x++)
                {
                    float v = srcImg.get(x, y);
                    if (v > peak)
                        peak = v;
                    v -= s.background;
                    mass += v;
                    center_x += x * v;
                    center_y += y * v;
                }
            star.x = center_x / mass;
            star.y = center_y / mass;
            star.background = s.background;
            star.peak = peak;
            if ((s.x < range) || (s.x >= w - range) || (s.y < range) || (s.y >= h - range))
            {
                star.peak = 0.0f;
                stars.Append(star);
                continue;
            }
            // Calculate size
            Array<float> x_values(range * 2 + 1), y_values(range * 2 + 1);
            for (int i = -range; i <= range; i++)
            {
                x_values[i + range] = srcImg.getBilinear(star.x + i, star.y) - star.background;
                y_values[i + range] = srcImg.getBilinear(star.x, star.y + i) - star.background;
            }
            star.sizeX = calcFwhm(x_values);
            star.sizeY = calcFwhm(y_values);
            stars.Append(star);
            dstImg.set(1.0f, star.x + 0.5f, star.y + 0.5f);
        }
    }

public:
    explicit StarDetectionThread(int id, LuckyIntegrationInstance* instance)
        : ImageThread(id, instance)
    {
        m_instance->m_starDetections.Clear();
    }

    virtual ~StarDetectionThread()
    {
    }

    void process(NativeImage& dstImage, const NativeImage& srcImage, int imageIdx) override
    {
        if ((m_instance->p_routine == LIRoutine::StarDetectionPreview) && (imageIdx >= 1))
            return;
        NativeImage correctedImg;
        bool corrected = false;
        if (imageIdx == 0)
        {
            getBackground(m_instance->m_backgroundImage, srcImage);
            cosmeticCorrection(correctedImg, srcImage, imageIdx > 0);
            corrected = true;
        }
        if (imageIdx == 0)
        {
            Array<Star> stars;
            starDetection(stars, m_instance->m_starDetectionPreviewImage, corrected ? correctedImg : srcImage);
            m_instance->m_starDetectionLock.Lock();
            m_instance->m_starDetections.Append(stars);
            m_instance->m_starDetectionLock.Unlock();
        }
        else
        {
            // Wait for previous frame
            while (1)
            {
                m_instance->m_starDetectionLock.Lock();
                size_t n = m_instance->m_starDetections.Length();
                m_instance->m_starDetectionLock.Unlock();
                if (imageIdx <= n)
                    break;
                else
                    Sleep(1);
            }
            m_instance->m_starDetectionLock.Lock();
            const Array<Star>& prevStars = m_instance->m_starDetections[size_t(imageIdx - 1)];
            m_instance->m_starDetectionLock.Unlock();
            Array<Star> stars;
            starMovement(stars, m_instance->m_starMovementImage, prevStars, corrected ? correctedImg : srcImage);
            m_instance->m_starDetectionLock.Lock();
            if (m_instance->m_starDetections.Length() < size_t(imageIdx + 1))
                m_instance->m_starDetections.Resize(size_t(imageIdx + 1));
            m_instance->m_starDetections[imageIdx] = stars;
            m_instance->m_starDetectionLock.Unlock();
        }
    }
};

class ImageIntegrationThread : public ImageThread
{
private:
    NativeImage m_localIntegration;
    int m_numTotalImages;
    int m_numIntegratedImages;
    double m_totalTimeMs;

    void integrate(const NativeImage& srcImage, int imageIdx)
    {
        const auto& stars = m_instance->m_starDetections[imageIdx];
        const auto& stars0 = m_instance->m_starDetections[0];
        F32Point displacement(0.0f, 0.0f);
        float starSizeX = 0.0f, starSizeY = 0.0f;
        for (int i = 0; i < stars.Length(); i++)
        {
            if (stars[i].peak == 0.0f)
                continue;
            displacement.x += stars[i].x - stars0[i].x;
            displacement.y += stars[i].y - stars0[i].y;
            starSizeX += stars[i].sizeX;
            starSizeY += stars[i].sizeY;
        }
        displacement.x /= stars.Length();
        displacement.y /= stars.Length();
        starSizeX /= stars.Length();
        starSizeY /= stars.Length();
        if (Max(starSizeX, starSizeY) > m_instance->p_starSizeRejectionThreshold)    // Rejection due to star size
            return;
        F32Point displacementLast(0.0f, 0.0f);
        if (imageIdx > 0)
        {
            const auto& starsLast = m_instance->m_starDetections[imageIdx - 1];
            for (int i = 0; i < stars.Length(); i++)
            {
                if (stars[i].peak == 0.0f)
                    continue;
                displacementLast.x += stars[i].x - starsLast[i].x;
                displacementLast.y += stars[i].y - starsLast[i].y;
            }
            displacementLast.x /= stars.Length();
            displacementLast.y /= stars.Length();
        }
        if (displacementLast.DistanceToOrigin() > m_instance->p_starMovementRejectionThreshold) // Rejection due to star movement
            return;

        m_numIntegratedImages++;

        int w = srcImage.width();
        int h = srcImage.height();
        NativeImage calibratedImage;
        calibratedImage.allocate<float>(w, h);
        calibratedImage.copy(srcImage);
        if (m_instance->m_hasDark)
        {
            calibratedImage.sub(m_instance->m_masterDarkImage);
            calibratedImage.addConst(m_instance->p_pedestal);
        }
        if (m_instance->m_hasFlat)
        {
            calibratedImage.div(m_instance->m_masterFlatImage);
            calibratedImage.mulConst(m_instance->m_masterFlatMean);
        }
        NativeImage registeredImage;
        registeredImage.allocate<float>(w, h);
        if (!m_instance->p_enableDigitalAO)
        {
            if (m_instance->p_interpolation == LIInterpolation::Nearest)
            {
                for (int y = 0; y < h; y++)
                    for (int x = 0; x < w; x++)
                        registeredImage.set(calibratedImage.getNearest(x + displacement.x, y + displacement.y), x, y);
            }
            else if (m_instance->p_interpolation == LIInterpolation::Bilinear)
            {
                for (int y = 0; y < h; y++)
                    for (int x = 0; x < w; x++)
                        registeredImage.set(calibratedImage.getBilinear(x + displacement.x, y + displacement.y), x, y);
            }
            else if (m_instance->p_interpolation == LIInterpolation::Lanczos3)
            {
                for (int y = 0; y < h; y++)
                    for (int x = 0; x < w; x++)
                        registeredImage.set(calibratedImage.getLanczos(x + displacement.x, y + displacement.y, 3), x, y);
            }
        }
        else
        {
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    F32Point displacement(0.0f, 0.0f);
                    float w0 = 0.0f;
                    for (int i = 0; i < stars.Length(); i++)
                    {
                        if (stars[i].peak == 0.0f)
                            continue;
                        float d2 = (stars[i].x - x) * (stars[i].x - x) + (stars[i].y - y) * (stars[i].y - y);
                        float w = 1.0f / (d2 + 1.0f);
                        displacement.x += (stars[i].x - stars0[i].x) * w;
                        displacement.y += (stars[i].y - stars0[i].y) * w;
                        w0 += w;
                    }
                    displacement.x /= w0;
                    displacement.y /= w0;
                    if (m_instance->p_interpolation == LIInterpolation::Nearest)
                        registeredImage.set(calibratedImage.getNearest(x + displacement.x, y + displacement.y), x, y);
                    else if (m_instance->p_interpolation == LIInterpolation::Bilinear)
                        registeredImage.set(calibratedImage.getBilinear(x + displacement.x, y + displacement.y), x, y);
                    else if (m_instance->p_interpolation == LIInterpolation::Lanczos3)
                        registeredImage.set(calibratedImage.getLanczos(x + displacement.x, y + displacement.y, 3), x, y);
                }
        }
        if (m_instance->p_registrationOnly)
        {
            // Save image
            ImageVariant registration;
            registration.CreateFloatImage();
            registration.AllocateData(registeredImage.width(), registeredImage.height());
            CopyMemory(static_cast<Image&>(*registration).PixelData(), registeredImage.data(), registeredImage.size());
            FileFormat format(".xisf", false/*toRead*/, true/*toWrite*/);
            FileFormatInstance file(format);
            ImageOptions options;
            options.bitsPerSample = 32;
            options.ieeefpSampleFormat = true;
            String filename = m_instance->p_registrationOutputPath + "\\" + File::ExtractName(m_globalData.inputFilenames[imageIdx]) + ".xisf";
            if (!file.Create(filename))
                throw CaughtException();
            if (!file.SetOptions(options))
                throw CaughtException();
            if (!file.WriteImage(registration))
                throw CaughtException();
            if (!file.Close())
                throw CaughtException();
        }
        else
        {
            m_localIntegration.add(registeredImage);
        }
    }

public:
    explicit ImageIntegrationThread(int id, LuckyIntegrationInstance* instance)
        : ImageThread(id, instance)
        , m_numTotalImages(0)
        , m_numIntegratedImages(0)
        , m_totalTimeMs(0.0)
    {
    }

    virtual ~ImageIntegrationThread()
    {
        if (m_id == 0)
        {
            m_instance->m_numTotalImages = m_instance->m_numIntegratedImages = 0;
            m_instance->m_averageProcessTimeMs = 0.0;
            if (!m_instance->p_registrationOnly)
            {
                m_instance->m_integration.allocate<float>(m_globalData.width, m_globalData.height);
                m_instance->m_integration.zero();
            }
        }

        m_instance->m_numTotalImages += m_numTotalImages;
        m_instance->m_numIntegratedImages += m_numIntegratedImages;
        if (m_localIntegration.isAllocated() && !m_instance->p_registrationOnly)
            m_instance->m_integration.add(m_localIntegration);
        m_instance->m_averageProcessTimeMs += m_totalTimeMs;
    }

    void process(NativeImage& dstImage, const NativeImage& srcImage, int imageIdx) override
    {
        if (imageIdx >= m_instance->m_starDetections.Length())
            throw Error(String().Format("Star detection for frame #%d does not exist.", imageIdx));

        m_numTotalImages++;
        if (!m_localIntegration.isAllocated() && !m_instance->p_registrationOnly)
        {
            m_localIntegration.allocate<float>(srcImage.width(), srcImage.height());
            m_localIntegration.zero();
        }

        double t0 = pcl::TimePoint::Now().MillisecondsSinceUNIXEpoch();
        integrate(srcImage, imageIdx);
        double t = pcl::TimePoint::Now().MillisecondsSinceUNIXEpoch();
        m_totalTimeMs += (t - t0);
    }
};

LuckyIntegrationInstance::LuckyIntegrationInstance(const MetaProcess* m)
    : ProcessImplementation(m)
    , p_routine(TheLIRoutineParameter->DefaultValueIndex())
    , p_approxFwhm(TheLIApproxFWHMParameter->DefaultValue())
    , p_minPeak(TheLIMinPeakParameter->DefaultValue())
    , p_saturationThreshold(TheLISaturationThresholdParameter->DefaultValue())
    , p_pedestal(TheLIPedestalParameter->DefaultValue())
    , p_enableDigitalAO(TheLIEnableDigitalAOParameter->DefaultValue())
    , p_starSizeRejectionThreshold(TheLIStarSizeRejectionThresholdParameter->DefaultValue())
    , p_starMovementRejectionThreshold(TheLIStarMovementRejectionThresholdParameter->DefaultValue())
    , p_interpolation(TheLIInterpolationParameter->DefaultValueIndex())
    , p_framePercentage(TheLIFramePercentageParameter->DefaultValue())
    , p_registrationOnly(TheLIRegistrationOnlyParameter->DefaultValue())
{
}

LuckyIntegrationInstance::LuckyIntegrationInstance(const LuckyIntegrationInstance& x)
    : ProcessImplementation(x)
{
    Assign(x);
}

void LuckyIntegrationInstance::Assign(const ProcessImplementation& p)
{
    const LuckyIntegrationInstance* x = dynamic_cast<const LuckyIntegrationInstance*>(&p);
    if (x != nullptr) {
        p_routine = x->p_routine;
        p_inputPath = x->p_inputPath;
        p_approxFwhm = x->p_approxFwhm;
        p_minPeak = x->p_minPeak;
        p_saturationThreshold = x->p_saturationThreshold;
        p_masterDark = x->p_masterDark;
        p_masterFlat = x->p_masterFlat;
        p_enableDigitalAO = x->p_enableDigitalAO;
        p_pedestal = x->p_pedestal;
        p_starSizeRejectionThreshold = x->p_starSizeRejectionThreshold;
        p_starMovementRejectionThreshold = x->p_starMovementRejectionThreshold;
        p_interpolation = x->p_interpolation;
        p_framePercentage = x->p_framePercentage;
        p_registrationOnly = x->p_registrationOnly;
        p_registrationOutputPath = x->p_registrationOutputPath;
    }
}

bool LuckyIntegrationInstance::IsHistoryUpdater(const View& view) const
{
    return false;
}

bool LuckyIntegrationInstance::CanExecuteOn(const View& view, String& whyNot) const
{
    whyNot = "LuckyIntegration can only be executed in the global context.";
    return false;
}

bool LuckyIntegrationInstance::CanExecuteGlobal(String& whyNot) const
{
    if (p_inputPath.IsEmpty())
    {
        whyNot = "Input directory is not specified.";
        return false;
    }

    return true;
}

bool LuckyIntegrationInstance::ExecuteGlobal()
{
    {
        String why;
        if (!CanExecuteGlobal(why))
            throw Error(why);

        if (!File::DirectoryExists(p_inputPath))
            throw Error("Input directory does not exist: " + p_inputPath);

        if (p_routine == LIRoutine::ImageIntegration)
        {
            if (!p_masterDark.path.IsEmpty())
            {
                if (!File::Exists(p_masterDark.path))
                    throw Error("Master dark file not found: " + p_masterDark.path);
            }

            if (!p_masterFlat.path.IsEmpty())
            {
                if (!File::Exists(p_masterFlat.path))
                    throw Error("Master flat file not found: " + p_masterFlat.path);
            }

            if (p_registrationOnly && p_registrationOutputPath.IsEmpty())
            {
                throw Error("Registration output directory is not specified.");
            }
        }
    }

    // allow the user to abort the calibration process.
    Console console;
    console.EnableAbort();

    if (p_routine == LIRoutine::StarDetectionPreview)
    {
        doStarDetectionPreview();
    }
    else if (p_routine == LIRoutine::StarDetectionAlignment)
    {
        doStarDetectionAlignment();
    }
    else if (p_routine == LIRoutine::ImageIntegration)
    {
        // load calibration images
        m_hasDark = m_hasFlat = false;
        if (!p_masterDark.path.IsEmpty())
        {
            Image image;
            LoadImage(image, p_masterDark.path);
            m_masterDarkImage.allocate<float>(image.Width(), image.Height());
            m_masterDarkImage.copyRaw(image.PixelData());
            m_hasDark = true;
        }
        if (!p_masterFlat.path.IsEmpty())
        {
            Image image;
            LoadImage(image, p_masterFlat.path);
            m_masterFlatMean = image.Mean();
            m_masterFlatImage.allocate<float>(image.Width(), image.Height());
            m_masterFlatImage.copyRaw(image.PixelData());
            m_hasFlat = true;
        }
        doImageIntegration();
    }

    return true;
}

void* LuckyIntegrationInstance::LockParameter(const MetaParameter* p, size_type tableRow)
{
    if (p == TheLIRoutineParameter)
        return &p_routine;
    if (p == TheLIInputPathParameter)
        return p_inputPath.Begin();
    if (p == TheLIApproxFWHMParameter)
        return &p_approxFwhm;
    if (p == TheLIMinPeakParameter)
        return &p_minPeak;
    if (p == TheLISaturationThresholdParameter)
        return &p_saturationThreshold;
    if (p == TheLIMasterDarkPathParameter)
        return p_masterDark.path.Begin();
    if (p == TheLIMasterFlatPathParameter)
        return p_masterFlat.path.Begin();
    if (p == TheLIPedestalParameter)
        return &p_pedestal;
    if (p == TheLIEnableDigitalAOParameter)
        return &p_enableDigitalAO;
    if (p == TheLIStarSizeRejectionThresholdParameter)
        return &p_starSizeRejectionThreshold;
    if (p == TheLIStarMovementRejectionThresholdParameter)
        return &p_starMovementRejectionThreshold;
    if (p == TheLIInterpolationParameter)
        return &p_interpolation;
    if (p == TheLIFramePercentageParameter)
        return &p_framePercentage;
    if (p == TheLIRegistrationOnlyParameter)
        return &p_registrationOnly;
    if (p == TheLIRegistrationOutputPathParameter)
        return p_registrationOutputPath.Begin();
    return 0;
}

bool LuckyIntegrationInstance::AllocateParameter(size_type sizeOrLength, const MetaParameter* p, size_type tableRow)
{
    if (p == TheLIInputPathParameter)
    {
        p_inputPath.Clear();
        if (sizeOrLength > 0)
            p_inputPath.SetLength(sizeOrLength);
    }
    else if (p == TheLIMasterDarkPathParameter)
    {
        p_masterDark.path.Clear();
        if (sizeOrLength > 0)
            p_masterDark.path.SetLength(sizeOrLength);
    }
    else if (p == TheLIMasterFlatPathParameter)
    {
        p_masterFlat.path.Clear();
        if (sizeOrLength > 0)
            p_masterFlat.path.SetLength(sizeOrLength);
    }
    else if (p == TheLIRegistrationOutputPathParameter)
    {
        p_registrationOutputPath.Clear();
        if (sizeOrLength > 0)
            p_registrationOutputPath.SetLength(sizeOrLength);
    }
    else
    {
        return false;
    }

    return true;
}

size_type LuckyIntegrationInstance::ParameterLength(const MetaParameter* p, size_type tableRow) const
{
    if (p == TheLIInputPathParameter)
        return p_inputPath.Length();
    else if (p == TheLIMasterDarkPathParameter)
        return p_masterDark.path.Length();
    else if (p == TheLIMasterFlatPathParameter)
        return p_masterFlat.path.Length();
    else if (p == TheLIRegistrationOutputPathParameter)
        return p_registrationOutputPath.Length();

    return 0;
}

void LuckyIntegrationInstance::doStarDetectionPreview()
{
    Console console;

    console.WriteLn("Detecting stars...");
    ImageThread::dispatch<StarDetectionThread>(this, 1);

    ImageVariant starDetectionPreview;
    starDetectionPreview.CreateFloatImage();
    starDetectionPreview.AllocateData(m_starDetectionPreviewImage.width(), m_starDetectionPreviewImage.height());
    CopyMemory(static_cast<Image&>(*starDetectionPreview).PixelData(), m_starDetectionPreviewImage.data(), m_starDetectionPreviewImage.size());

    String id = "StarDetection";
    ImageWindow w = ImageWindow(starDetectionPreview.Width(), starDetectionPreview.Height(), starDetectionPreview.NumberOfChannels(), starDetectionPreview.BitsPerSample(),
                                starDetectionPreview.IsFloatSample(), starDetectionPreview.IsColor(), true, id);
    if (w.IsNull())
        throw Error("Unable to create image window: " + id);
    w.MainView().Lock();
    w.MainView().Image().CopyImage(starDetectionPreview);
    w.MainView().Unlock();
    w.Show();
}

void LuckyIntegrationInstance::doStarDetectionAlignment()
{
    Console console;

    console.WriteLn("Detecting stars and calculating movement...");
    ImageThread::dispatch<StarDetectionThread>(this);

    if (m_starDetections.Length() == 0)
        throw Error("No star detected.");

    String xmlFilename = p_inputPath + "\\star_detections.xml";
    console.WriteLn(String("Writing detections to ") + xmlFilename + "...");

    XMLElement* e1 = new XMLElement("StarDetection", XMLAttributeList() << XMLAttribute("version", "1.0"));
    for (int i = 0; i < m_starDetections.Length(); i++)
    {
        XMLElement* e2 = new XMLElement(*e1, "Frame", XMLAttributeList() << XMLAttribute("id", String(i)));
        for (int j = 0; j < m_starDetections[i].Length(); j++)
        {
            const Star& s = m_starDetections[i][j];
            XMLElement* e3 = new XMLElement(*e2, "Star", XMLAttributeList() << XMLAttribute("id", String(s.id)) << XMLAttribute("x", String(s.x)) << XMLAttribute("y", String(s.y))
                                                                            << XMLAttribute("background", String(s.background)) << XMLAttribute("peak", String(s.peak))
                                                                            << XMLAttribute("sizeX", String(s.sizeX)) << XMLAttribute("sizeY", String(s.sizeY)));
        }
    }
    XMLDocument xml;
    xml.SetXML("1.0");
    xml.SetRootElement(e1);
    xml.EnableAutoFormatting();
    xml.SerializeToFile(xmlFilename);

    ImageVariant starMovementImage;
    starMovementImage.CreateFloatImage();
    starMovementImage.AllocateData(m_starMovementImage.width(), m_starMovementImage.height());
    CopyMemory(static_cast<Image&>(*starMovementImage).PixelData(), m_starMovementImage.data(), m_starMovementImage.size());

    String id = "StarMovement";
    ImageWindow w = ImageWindow(starMovementImage.Width(), starMovementImage.Height(), starMovementImage.NumberOfChannels(), starMovementImage.BitsPerSample(),
                                starMovementImage.IsFloatSample(), starMovementImage.IsColor(), true, id);
    if (w.IsNull())
        throw Error("Unable to create image window: " + id);
    w.MainView().Lock();
    w.MainView().Image().CopyImage(starMovementImage);
    w.MainView().Unlock();
    w.Show();
}

void LuckyIntegrationInstance::doImageIntegration()
{
    Console console;

    String xmlFilename = p_inputPath + "\\star_detections.xml";
    console.WriteLn(String("Reading detections from ") + xmlFilename + "...");

    m_starDetections.Clear();
    XMLDocument xml;
    xml.Parse(File::ReadTextFile(xmlFilename).UTF8ToUTF16());
 
    const XMLElement* e1 = xml.RootElement();
    if (e1->Name() != "StarDetection")
        throw Error("Unrecognized root element " + e1->Name());
    if (e1->AttributeValue("version") != "1.0")
        throw Error("Wrong version " + e1->AttributeValue("version"));
    for (const XMLElement& e2 : e1->ChildElements())
    {
        if (e2.Name() != "Frame")
            throw Error("Unrecognized element " + e2.Name());
        Array<Star> stars;
        for (const XMLElement& e3 : e2.ChildElements())
        {
            if (e3.Name() != "Star")
                throw Error("Unrecognized element " + e3.Name());
            Star s;
            s.id = e3.AttributeValue("id").ToInt();
            s.x = e3.AttributeValue("x").ToFloat();
            s.y = e3.AttributeValue("y").ToFloat();
            s.background = e3.AttributeValue("background").ToFloat();
            s.peak = e3.AttributeValue("peak").ToFloat();
            s.sizeX = e3.AttributeValue("sizeX").ToFloat();
            s.sizeY = e3.AttributeValue("sizeY").ToFloat();
            stars.Append(s);
        }
        m_starDetections.Append(stars);
    }

    console.WriteLn(String().Format("Got %d frames of star detections. Each frame has %d stars.", m_starDetections.Length(), m_starDetections[0].Length()));

    if (p_registrationOnly)
        console.WriteLn("Running image registration...");
    else
        console.WriteLn("Running image integration...");

    ImageThread::dispatch<ImageIntegrationThread>(this);
    if ((m_numIntegratedImages > 0) && !p_registrationOnly)
        m_integration.divConst(m_numIntegratedImages);
    console.WriteLn(String().Format("Rejection percentage: %.3f%%", 100.0f - 100.0f * m_numIntegratedImages / m_numTotalImages));

    m_averageProcessTimeMs /= m_numIntegratedImages;
    console.WriteLn(String().Format("Average processing time per image: %.3lfms", m_averageProcessTimeMs));

    if (p_registrationOnly)
        return;

    if (!m_integration.isAllocated())
        throw Error("No integration result.");

    m_integration.clip();

    ImageVariant integration;
    integration.CreateFloatImage();
    integration.AllocateData(m_integration.width(), m_integration.height());
    CopyMemory(static_cast<Image&>(*integration).PixelData(), m_integration.data(), m_integration.size());

    String id = "Integration";
    ImageWindow w = ImageWindow(integration.Width(), integration.Height(), integration.NumberOfChannels(), integration.BitsPerSample(),
                                integration.IsFloatSample(), integration.IsColor(), true, id);
    if (w.IsNull())
        throw Error("Unable to create image window: " + id);
    w.MainView().Lock();
    w.MainView().Image().CopyImage(integration);
    w.MainView().Unlock();
    w.Show();
}

}	// namespace pcl