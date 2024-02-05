#pragma once
#include <string>

namespace Core {
    class Path;

    template<typename Container>
    class PathBuffer;

    using HeapPathBuffer = PathBuffer<std::string>;

    class PathPart {
        std::string mUtf8StdString;
    public:
        PathPart() = default;
        PathPart(std::string&& str) : mUtf8StdString(str) {}
        PathPart(const std::string& str) : mUtf8StdString(str) {}
        //PathPart(gsl::not_null<const char*> pCStr) : mUtf8StdString(pCStr) {}
        //PathPart(gsl::not_null<const char*> pCStr, size_t size) : mUtf8StdString(pCStr, size) {}
        PathPart(const char* pCStr) : mUtf8StdString(pCStr) {}
        PathPart(const char* pCStr, size_t size) : mUtf8StdString(pCStr, size) {}
        PathPart(const Core::Path& path);

        template<typename Container>
        PathPart(const Core::PathBuffer<Container>& pathBuffer);
        
        const char* getUtf8CString() const {
            return mUtf8StdString.c_str();
        }
        const std::string& getUtf8StdString() const {
            return mUtf8StdString;
        }
        size_t size() const {
            return mUtf8StdString.size();
        }
        bool empty() const {
            return mUtf8StdString.empty();
        }
        bool operator<(const Core::PathPart& rhs) const {
            return mUtf8StdString < rhs.mUtf8StdString;
        }
        bool operator==(const Core::PathPart& rhs) const {
            return mUtf8StdString == rhs.mUtf8StdString;
        }
        bool operator!=(const Core::PathPart& rhs) const {
            return mUtf8StdString != rhs.mUtf8StdString;
        }
    };

    template <typename Container>
    class PathBuffer {
    public:
        static const PathBuffer<Container> EMPTY;
    private:
        Container mContainer;
    public:
        PathBuffer() = default;
        PathBuffer(Core::PathBuffer<Container>&& rhs) noexcept : mContainer(std::move(rhs.mContainer)) {}
        PathBuffer(const Core::PathBuffer<Container>& rhs) : mContainer(rhs.mContainer) {}
        PathBuffer(const std::string& s) : mContainer(s) {}
        PathBuffer(const char* s) : mContainer(s) {}
        //PathBuffer(gsl::string_span stringSpan);
        PathBuffer(const Core::Path& s);
        PathBuffer(Container&& container) : mContainer(container) {}

        Core::PathBuffer<Container>& operator=(Core::PathBuffer<Container>&& rhs);
        Core::PathBuffer<Container>& operator=(const Core::Path& s);
        Core::PathBuffer<Container>& operator=(const Core::PathBuffer<Container>& rhs);
        Core::PathBuffer<Container>& operator+=(const char* str);
        bool operator==(const Core::Path& s) const;
        bool operator!=(const Core::Path& s) const;
        char const* getUtf8CString() const {
            return mContainer.c_str();
        }
        //gsl::string_span getUtf8StringSpan() const;
        size_t size() const {
            return mContainer.size();
        }
        void clear() {
            mContainer.clear();
        }
        void reserve(size_t s) {
            mContainer.reserve(s);
        }
        bool empty() const {
            return mContainer.empty();
        }
        void push_back(char c) {
            mContainer.push_back(c);
        }
        Container&& moveContainer();
        const Container& getContainer() const {
            return mContainer;
        }
    };

    class Path {
    public:
        static const Core::Path EMPTY;
    private:
        Core::PathPart mPathPart;
    public:
        Path() = default;
        Path(std::string&& str) : mPathPart(str) {}
        Path(const std::string& str) : mPathPart(str) {}
        Path(const char* str) : mPathPart(str) {}
        Path(const char* str, size_t size) : mPathPart(str, size) {}
        Path(const Core::PathPart& a2) : mPathPart(a2) {}
        Path(const Core::Path& rhs) : mPathPart(rhs.mPathPart) {}
        Path(Core::Path&& path) noexcept : mPathPart(path.mPathPart) {}

        template<typename Container>
        Path(const Core::PathBuffer<Container>& pathBuffer) : mPathPart(pathBuffer.getUtf8CString(), pathBuffer.size()) {}

        bool isDotOrDotDot() const;
        Core::Path& operator=(const Core::Path& rhs);
        Core::Path& operator=(Core::Path&& rhs);
        bool operator<(const Core::Path& rhs) const;
        bool operator==(const Core::Path& rhs) const;
        bool operator!=(const Core::Path& rhs) const;
        char const* getUtf8CString() const {
            return mPathPart.getUtf8CString();
        }
        //gsl::string_span getUtf8StringSpan() const;
        const std::string& getUtf8StdString() const {
            return mPathPart.getUtf8StdString();
        }
        size_t size() const {
            return mPathPart.size();
        }
        bool empty() const {
            return mPathPart.empty();
        }
    };
}

inline Core::PathPart::PathPart(const Core::Path& path) : mUtf8StdString(path.getUtf8StdString()) {}

template<typename Container>
inline Core::PathPart::PathPart(const Core::PathBuffer<Container>& pathBuffer) : mUtf8StdString(pathBuffer.getContainer()) {}

template <typename Container>
inline Core::PathBuffer<Container>::PathBuffer(const Core::Path& s) : mContainer(s.getUtf8StdString()) {}
