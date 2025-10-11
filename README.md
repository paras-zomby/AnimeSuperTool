# AnimeSuperTool

A C++ tool for anime super resolution and video frame interpolation. This tool leverages state-of-the-art deep learning models to enhance video quality by increasing resolution and frame rate.

## Features

*   **Video Super-Resolution**: Upscale video resolution using the Real-CUGAN model.
*   **Video Frame Interpolation**: Increase video frame rate using the RIFE or IFRNet models.
*   **GPU Acceleration**: Utilizes Vulkan for high-performance inference with the ncnn library.
*   **Batch Processing**: Process a whole directory of videos in one go.

## Dependencies

To build and run AnimeSuperTool, you will need the following dependencies:

*   **CMake**: For building the project.
*   **C++ Compiler**: A compiler with C++11 support or newer (e.g., GCC, Clang, MSVC).
*   **ncnn**: A high-performance neural network inference framework. It is included as a git submodule.
*   **FFmpeg**: For decoding and encoding video files.
*   **Boost**: Specifically `boost::filesystem` for directory operations.
*   **Vulkan SDK**: Required by ncnn for GPU acceleration.

## Setup and Building

1.  **Clone the repository:**
    ```bash
    git clone --recursive <repository-url>
    cd AnimeSuperTool
    ```
    If you have already cloned the repository without the `--recursive` flag, you can initialize the submodules with:
    ```bash
    git submodule update --init --recursive
    ```

2.  **Install Dependencies:**
    *   **FFmpeg, Boost, Vulkan SDK**: Install these libraries using your system's package manager (e.g., `apt`, `brew`, `pacman`) or by downloading them from their official websites. Make sure they are discoverable by CMake (e.g., by setting environment variables or installing them to a standard location).

3.  **Build with CMake:**
    Create a build directory and run CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
    On Windows, you might want to specify a generator:
    ```bash
    mkdir build
    cd build
    cmake -G "Visual Studio 17 2022" ..
    cmake --build . --config Release
    ```

## Usage

After building, the executable will be located in the `build` directory.

Currently, the input path and processing parameters are hardcoded in `src/main.cpp`. You will need to modify the `main` function to set your desired input path and parameters.

```cpp
int main() {
    // Hardcoded path to a video file or a directory of videos
    std::string path = "D:/Animes/YourVideoFolder";

    // Configuration parameters
    Params param;
    param.zoom_scale = 2;          // Super-resolution scale (e.g., 2x)
    param.frame_ratio_scale = 3;   // Frame interpolation scale (e.g., 3x)
    param.denoise = false;         // Enable/disable denoising
    param.num_threads_pergpu = 2;  // Number of threads per GPU
    param.tile_num = 2;            // Tiling for memory management
    param.gpu = 0;                 // GPU device ID
    // ... other parameters
}
```

Once configured, run the executable from the root of the repository to ensure the model paths (`../models`) are resolved correctly.

The processed videos will be saved in the same directory as the input files, with a prefix like `[AST 2x@3x]` added to the filename.

## Credits

This project is built upon the great work of others:

-   [ncnn](https://github.com/Tencent/ncnn)
-   [ncnn-realcugan](https://github.com/nihui/realcugan-ncnn-vulkan)
-   [ncnn-rife](https://github.com/nihui/rife-ncnn-vulkan)
-   The progress bar implementation is based on [progresscpp](https://github.com/p-ranav/indicators).