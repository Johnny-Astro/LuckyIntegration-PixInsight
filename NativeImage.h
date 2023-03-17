#pragma once

template<typename T>
class NativeImageData;

class NativeImageDataBase
{
protected:
    NativeImageDataBase() {}
    virtual ~NativeImageDataBase() {};
    virtual void* data() = 0;
    virtual const void* data() const = 0;
    virtual void zero() = 0;
    virtual float get(int, int) const = 0;
    virtual void set(float, int, int) = 0;
    virtual void pixelAdd(float, int, int) = 0;
    virtual void copy(const NativeImageDataBase*) = 0;
    virtual void add(const NativeImageDataBase*) = 0;
    virtual void addConst(float c) = 0;
    virtual void sub(const NativeImageDataBase*) = 0;
    virtual void rsub(const NativeImageDataBase*) = 0;
    virtual void mul(const NativeImageDataBase*) = 0;
    virtual void mulConst(float c) = 0;
    virtual void div(const NativeImageDataBase*) = 0;
    virtual void divConst(float c) = 0;
    virtual void clip() = 0;

    int m_numPixels = 0;

    friend class NativeImageData<uint8_t>;
    friend class NativeImageData<uint16_t>;
    friend class NativeImageData<uint32_t>;
    friend class NativeImageData<float>;
    friend class NativeImage;
};

template<typename T>
class NativeImageData : public NativeImageDataBase
{
private:
    T* m_data;
    int m_pitch;

    explicit NativeImageData(int n, int pitch)
        : m_data(new T[n])
        , m_pitch(pitch)
    {
        m_numPixels = n;
    }

    virtual ~NativeImageData()
    {
        delete[] m_data;
    }

    void* data() override
    {
        return reinterpret_cast<void*>(m_data);
    }

    const void* data() const override
    {
        return reinterpret_cast<const void*>(m_data);
    }

    void zero() override
    {
        for (int i = 0; i < m_numPixels; i++)
            m_data[i] = 0;
    }

    float get(int x, int y) const override
    {
        return static_cast<float>(m_data[y * m_pitch + x]);
    }

    void set(float v, int x, int y) override
    {
        m_data[y * m_pitch + x] = static_cast<T>(v);
    }

    void pixelAdd(float v, int x, int y) override
    {
        m_data[y * m_pitch + x] += static_cast<T>(v);
    }

#define COPY_IF_TYPE_IS(datatype)   \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] = dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i];   \
        return; \
    }

    void copy(const NativeImageDataBase* src) override
    {
        delete[] m_data;
        m_data = new T[src->m_numPixels];
        m_numPixels = src->m_numPixels;
        if (dynamic_cast<const NativeImageData<T>*>(src))
        {
            CopyMemory(m_data, dynamic_cast<const NativeImageData<T>*>(src)->m_data, m_numPixels * sizeof(T));
            return;
        }
        COPY_IF_TYPE_IS(uint8_t);
        COPY_IF_TYPE_IS(uint16_t);
        COPY_IF_TYPE_IS(uint32_t);
        COPY_IF_TYPE_IS(float);
    }

#define ADD_IF_TYPE_IS(datatype)    \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] += dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i];   \
        return; \
    }

    void add(const NativeImageDataBase* src) override
    {
        ADD_IF_TYPE_IS(uint8_t);
        ADD_IF_TYPE_IS(uint16_t);
        ADD_IF_TYPE_IS(uint32_t);
        ADD_IF_TYPE_IS(float);
    }

    void addConst(float c) override
    {
        for (int i = 0; i < m_numPixels; i++)
            m_data[i] += c;
    }

#define SUB_IF_TYPE_IS(datatype)    \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] -= dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i];   \
        return; \
    }

    void sub(const NativeImageDataBase* src) override
    {
        SUB_IF_TYPE_IS(uint8_t);
        SUB_IF_TYPE_IS(uint16_t);
        SUB_IF_TYPE_IS(uint32_t);
        SUB_IF_TYPE_IS(float);
    }

#define RSUB_IF_TYPE_IS(datatype)    \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] = dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i] - m_data[i]; \
        return; \
    }

    void rsub(const NativeImageDataBase* src) override
    {
        RSUB_IF_TYPE_IS(uint8_t);
        RSUB_IF_TYPE_IS(uint16_t);
        RSUB_IF_TYPE_IS(uint32_t);
        RSUB_IF_TYPE_IS(float);
    }

#define MUL_IF_TYPE_IS(datatype)    \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] *= dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i];   \
        return; \
    }

    void mul(const NativeImageDataBase* src) override
    {
        MUL_IF_TYPE_IS(uint8_t);
        MUL_IF_TYPE_IS(uint16_t);
        MUL_IF_TYPE_IS(uint32_t);
        MUL_IF_TYPE_IS(float);
    }

    void mulConst(float c) override
    {
        for (int i = 0; i < m_numPixels; i++)
            m_data[i] *= c;
    }

#define DIV_IF_TYPE_IS(datatype)    \
    if (dynamic_cast<const NativeImageData<datatype>*>(src))  \
    {   \
        for (int i = 0; i < m_numPixels; i++)   \
            m_data[i] /= dynamic_cast<const NativeImageData<datatype>*>(src)->m_data[i];   \
        return; \
    }

    void div(const NativeImageDataBase* src) override
    {
        DIV_IF_TYPE_IS(uint8_t);
        DIV_IF_TYPE_IS(uint16_t);
        DIV_IF_TYPE_IS(uint32_t);
        DIV_IF_TYPE_IS(float);
    }

    void divConst(float c) override
    {
        for (int i = 0; i < m_numPixels; i++)
            m_data[i] /= c;
    }

    void clip() override
    {
        for (int i = 0; i < m_numPixels; i++)
        {
            auto v = m_data[i];
            if (v < 0.0f)
                m_data[i] = 0.0f;
            else if (v > 1.0f)
                m_data[i] = 1.0f;
        }
    }

    friend class NativeImageData<uint8_t>;
    friend class NativeImageData<uint16_t>;
    friend class NativeImageData<uint32_t>;
    friend class NativeImageData<float>;
    friend class NativeImage;
};

class NativeImage
{
private:
    NativeImageDataBase* m_image;
    int m_width;
    int m_height;
    int m_depth;
    int m_size;

    static float sinc(float x)
    {
        x *= 3.14159265358979f;
        return (x > 1.0e-07f) ? sin(x) / x : 1.0f;
    }

    static float lanczos(float x, int n)
    {
        if (x < 0.0f)
            x = -x;
        if (x < n)
            return sinc(x) * sinc(x / n);
        return 0;
    }

    void lanczosInterpolateRow(float& sp, float& sn, float& wp, float& wn, int n, int x0, int y0, float* Lx, float Ly) const
    {
        int j, k;
        for (j = -n + 1, k = 0; j <= n; ++j, ++k)
        {
            int x = x0 + j;
            float L = Lx[k] * Ly;
            float s = m_image->get(x, y0) * L;
            if (s < 0)
            {
                sn -= s;
                wn -= L;
            }
            else
            {
                sp += s;
                wp += L;
            }
        }
    }

public:
    NativeImage()
        : m_image(nullptr)
        , m_width(0)
        , m_height(0)
        , m_depth(0)
        , m_size(0)
    {
    }

    virtual ~NativeImage()
    {
        if (m_image)
            delete m_image;
    }

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    int depth() const
    {
        return m_depth;
    }

    int size() const
    {
        return m_size;
    }

    template<typename T>
    void allocate(int w, int h, int sz = 0)
    {
        if (m_image)
            delete m_image;
        if (sz == 0)
            sz = w * h * sizeof(T);
        m_image = new NativeImageData<T>(sz / sizeof(T), w);
        m_width = w;
        m_height = h;
        m_depth = sizeof(T) * 8;
        m_size = sz;
    }

    bool isAllocated() const
    {
        return (m_image != nullptr);
    }

    float get(int x, int y) const
    {
        return m_image->get(x, y);
    }

    void set(float v, int x, int y)
    {
        m_image->set(v, x, y);
    }

    void pixelAdd(float v, int x, int y)
    {
        m_image->pixelAdd(v, x, y);
    }

    float getNearest(float x, float y) const
    {
        return m_image->get(int(x + 0.5f), int(y + 0.5f));
    }

    float getBilinear(float x, float y) const
    {
        if (x < 1)
            x = 1;
        else if (x > m_width - 2)
            x = m_width - 2;
        if (y < 1)
            y = 1;
        else if (y > m_height - 2)
            x = m_height - 2;
        // Calculate the coordinates of the 4 nearest integer pixels
        int x1 = int(x);
        int y1 = int(y);
        int x2 = x1 + 1;
        int y2 = y1 + 1;
        // Calculate the distances to the nearest pixels
        float dx1 = x - x1;
        float dy1 = y - y1;
        float dx2 = x2 - x;
        float dy2 = y2 - y;
        float value = dx2 * dy2 * m_image->get(x1, y1) +
                      dx1 * dy2 * m_image->get(x2, y1) +
                      dx2 * dy1 * m_image->get(x1, y2) +
                      dx1 * dy1 * m_image->get(x2, y2);

        return value;
    }

    float getLanczos(float x, float y, int n) const
    {
        if (x < n)
            x = n;
        else if (x > m_width - n - 1)
            x = m_width - n - 1;
        if (y < n)
            y = n;
        else if (y > m_height - n - 1)
            x = m_height - n - 1;

        int x0 = int(x);
        int y0 = int(y);

        float sp = 0.0f;   // positive filter values
        float sn = 0.0f;   // negative filter values
        float wp = 0.0f;   // positive filter weight
        float wn = 0.0f;   // negative filter weight

        // Interpolation increments
        float dx = x - x0;
        float dy = y - y0;

        // Precalculate horizontal filter values
        float Lx[10];
        for (int j = -n + 1, k = 0; j <= n; ++j, ++k)
            Lx[k] = lanczos(j - dx, n);

        for (int i = -n + 1; i <= n; ++i)
        {
            int y = y0 + i;
            lanczosInterpolateRow(sp, sn, wp, wn, n, x0, y0, Lx, lanczos(i - dy, n));
        }

        // Weighted convolution
        return (sp - sn) / (wp - wn);
    }

    void zero()
    {
        m_image->zero();
    }

    void copyRaw(const void* buf)
    {
        CopyMemory(m_image->data(), buf, m_size);
    }

    void copy(const NativeImage& srcImage)
    {
        m_width = srcImage.m_width;
        m_height = srcImage.m_height;
        m_depth = srcImage.m_depth;
        m_size = srcImage.m_size;
        m_image->copy(srcImage.m_image);
    }

    void add(const NativeImage& srcImage)
    {
        m_image->add(srcImage.m_image);
    }

    void sub(const NativeImage& srcImage)
    {
        m_image->sub(srcImage.m_image);
    }

    void rsub(const NativeImage& srcImage)
    {
        m_image->rsub(srcImage.m_image);
    }

    void mul(const NativeImage& srcImage)
    {
        m_image->mul(srcImage.m_image);
    }

    void div(const NativeImage& srcImage)
    {
        m_image->div(srcImage.m_image);
    }

    void addConst(float c)
    {
        m_image->addConst(c);
    }

    void subConst(float c)
    {
        m_image->addConst(-c);
    }

    void mulConst(float c)
    {
        m_image->mulConst(c);
    }

    void divConst(float c)
    {
        m_image->divConst(c);
    }

    void clip()
    {
        m_image->clip();
    }

    void* data()
    {
        return m_image->data();
    }

    const void* data() const
    {
        return m_image->data();
    }
};