#pragma once
#include <Geode/Geode.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <eclipse.ffmpeg-api/include/render_settings.hpp>
#include <eclipse.ffmpeg-api/include/recorder.hpp>
#include <queue>

using namespace geode::prelude;
using namespace ffmpeg;

namespace dashcam {
    class Utils {
    public:
        static Utils* shared() {
            static Utils instance;
            return &instance;
        }

        bool screenshot() {
            log::info("Screenshot event triggered");

            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int x0 = viewport[0];
            int y0 = viewport[1];
            int width = viewport[2];
            int height = viewport[3];

            if (width <= 0 || height <= 0) {
                log::warn("Invalid viewport size: {}x{}", width, height);
                return false;
            }

            int bufferLen = width * height * 4;
            unsigned char* buffer = new (std::nothrow) unsigned char[bufferLen];
            if (!buffer) {
                log::warn("Failed to allocate memory for screenshot buffer");
                return false;
            }

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                log::warn("glReadPixels error: {}", err);
                delete[] buffer;
                return false;
            }

            std::vector<unsigned char> data(buffer, buffer + bufferLen);
            delete[] buffer;

            static int iteration = 0;
            static std::time_t lastSecond = 0;

            std::time_t now = std::time(nullptr);
            if (now == (std::time_t)(-1)) {
                log::warn("Failed to get current time");
                return false;
            }

            std::tm local_tm = fmt::localtime(now);

            if (now != lastSecond) {
                lastSecond = now;
                iteration = 0;
            } else {
                iteration++;
            }

            auto screenshotsDir = Mod::get()->getSaveDir() / "screenshots";
            std::error_code ec;
            if (!std::filesystem::create_directories(screenshotsDir, ec) && ec) {
                log::warn("Failed to create screenshots directory: {}", ec.message());
                return false;
            }

            std::string filename = fmt::format(
                "screenshot_{:%Y-%m-%d_%H-%M-%S}_{}.png",
                local_tm, iteration
            );

            auto path = screenshotsDir / filename;

            std::thread([data = std::move(data), width, height, path]() mutable {
                std::vector<unsigned char> flipped(data.size());
                for (int y = 0; y < height; ++y) {
                    memcpy(
                        &flipped[y * width * 4],
                        &data[(height - 1 - y) * width * 4],
                        width * 4
                    );
                }

                int success = stbi_write_png(
                    path.string().c_str(),
                    width,
                    height,
                    4,
                    flipped.data(),
                    width * 4
                );

                if (!success) {
                    log::warn("Failed to save screenshot with stb_image_write to {}", path.string());
                    Notification::create("Unable to save screenshot, please ignore next notification.", NotificationIcon::Error)->show();
                } else {
                    log::info("Screenshot saved to {}", path.string());
                }
            }).detach();

            return true;
        }

        void pushFrameFromMainThread() {
            if (!m_recording) return;

            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int width = viewport[2];
            int height = viewport[3];

            if (width == 0 || height == 0) {
                log::warn("Viewport size is zero, skipping frame");
                return;
            }

            std::vector<uint8_t> frame(width * height * 4);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame.data());

            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                log::warn("glReadPixels error in pushFrameFromMainThread: {}", err);
                return;
            }

            std::vector<uint8_t> rgb0Frame(width * height * 4);
            for (int y = 0; y < height; ++y) {
                const uint8_t* src = &frame[(height - 1 - y) * width * 4];
                uint8_t* dst = &rgb0Frame[y * width * 4];
                for (int x = 0; x < width; ++x) {
                    dst[x * 4 + 0] = src[x * 4 + 0];
                    dst[x * 4 + 1] = src[x * 4 + 1];
                    dst[x * 4 + 2] = src[x * 4 + 2];
                    dst[x * 4 + 3] = 0;
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_frameQueue.push(std::move(rgb0Frame));
            }
            m_cv.notify_one();
        }

        bool startRecording() {
            if (m_recording) {
                log::warn("Already recording.");
                return false;
            }

            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int width = viewport[2];
            int height = viewport[3];

            if (width == 0 || height == 0) {
                log::warn("Viewport size is zero, cannot start recording.");
                return false;
            }

            m_width = width;
            m_height = height;

            ffmpeg::RenderSettings settings;
            settings.m_pixelFormat = PixelFormat::RGB0;
            settings.m_width = width;
            settings.m_height = height;
            settings.m_fps = 60;
            settings.m_bitrate = 30'000'000;
            settings.m_codec = "h264_nvenc";
            settings.m_outputFile = (Mod::get()->getSaveDir() / "recordings" / "recorded_video.mp4").string();

            std::error_code ec;
            if (!std::filesystem::create_directories(Mod::get()->getSaveDir() / "recordings", ec) && ec) {
                log::warn("Failed to create recordings directory: {}", ec.message());
                return false;
            }

            if (!m_recorder.init(settings)) {
                log::error("Failed to initialize recorder");
                return false;
            }

            m_recording = true;
            m_stop = false;

            log::info("Recording thread started");

            m_thread = std::thread([this]() {
                while (!m_stop) {
                    std::vector<uint8_t> frame;

                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_cv.wait(lock, [this] { return !m_frameQueue.empty() || m_stop; });

                        if (!m_frameQueue.empty()) {
                            frame = std::move(m_frameQueue.front());
                            m_frameQueue.pop();
                        }
                    }

                    if (!frame.empty()) {
                        auto res = m_recorder.writeFrame(frame);
                        if (!res) {
                            log::warn("Failed to write frame: {}", res.err());
                        }
                    }
                }

                m_recorder.stop();
                m_recording = false;
                log::info("Recording stopped.");
            });

            return true;
        }

        bool stopRecording() {
            if (!m_recording) return false;
            m_stop = true;
            m_cv.notify_all();
            if (m_thread.joinable()) m_thread.join();
            return true;
        }

        bool isRecording() {
            return m_recording.load();
        }

    private:
        ffmpeg::Recorder m_recorder;
        std::thread m_thread;
        std::atomic<bool> m_recording = false;
        std::atomic<bool> m_stop = false;
        int m_width = 0;
        int m_height = 0;

        std::mutex m_mutex;
        std::queue<std::vector<uint8_t>> m_frameQueue;
        std::condition_variable m_cv;
    };
}
