#pragma once
#include <Geode/Geode.hpp>
#include <eclipse.ffmpeg-api/include/render_settings.hpp>
#include <eclipse.ffmpeg-api/include/recorder.hpp>
#include <queue>

using namespace geode::prelude;
using namespace ffmpeg;

namespace dashcam {
    // enum ShaderType {
    //     Invert,
    //     Pixelated,
    //     Blur
    // };

    class Utils {
    public:
        static Utils* shared();

        // void renderSceneToTexture();
        // void applyShader(ShaderType shader);
        bool screenshot();
        void setScreenshots(std::vector<std::tuple<int, std::filesystem::path, std::string>>);
        std::vector<std::tuple<int, std::filesystem::path, std::string>> getScreenshots();

    private:
        CCRenderTexture* rt = nullptr;

        ffmpeg::Recorder m_recorder;
        std::thread m_thread;
        std::atomic<bool> m_recording = false;
        std::atomic<bool> m_stop = false;
        int m_width = 0;
        int m_height = 0;

        std::mutex m_mutex;
        std::queue<std::vector<uint8_t>> m_frameQueue;
        std::condition_variable m_cv;
        std::vector<std::tuple<int, std::filesystem::path, std::string>> m_screenshots;
    };
}
