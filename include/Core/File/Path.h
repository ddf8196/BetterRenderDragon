#pragma once
#include <string>

namespace Core {
    template <typename T>
    class PathBuffer {
    public:
	    static PathBuffer<T> EMPTY;
	    T mContainer;

        PathBuffer(const char* path) : mContainer(path) {
        
        }
    };


    class PathPart {
    public:
	    std::string mUtf8StdString;
    };

    class Path {
    public:
	    static Path EMPTY;
	    PathPart mPathPart;

        Path() {}

        Path(std::string path) {
            mPathPart.mUtf8StdString = path;
        }

        template<typename T>
        Path(const Core::PathBuffer<T>& path) {

        }
    };
}
