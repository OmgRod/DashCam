#include <Geode/Geode.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace geode::prelude;

namespace dashcam {
    class Utils {
    public:
        inline bool screenshot() {
            log::info("Screenshot event triggered");

            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int x0 = viewport[0];
            int y0 = viewport[1];
            int width = viewport[2];
            int height = viewport[3];
            int bufferLen = width * height * 4;

            unsigned char* buffer = new unsigned char[bufferLen];
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                log::warn("glReadPixels error: {}", err);
                delete[] buffer;
                return false;
            }

            unsigned char* flipped = new unsigned char[bufferLen];
            for (int y = 0; y < height; ++y) {
                memcpy(
                    flipped + y * width * 4,
                    buffer + (height - 1 - y) * width * 4,
                    width * 4
                );
            }

            static int iteration = 0;
            static std::time_t lastSecond = 0;

            std::time_t now = std::time(nullptr);
            std::tm local_tm = fmt::localtime(now);

            if (now != lastSecond) {
                lastSecond = now;
                iteration = 0;
            } else {
                iteration++;
            }

            auto screenshotsDir = Mod::get()->getSaveDir() / "screenshots";
            std::filesystem::create_directories(screenshotsDir);

            std::string filename = fmt::format(
                "screenshot_{:%Y-%m-%d_%H-%M-%S}_{}.png",
                local_tm, iteration);

            auto path = screenshotsDir / filename;

            int success = stbi_write_png(
                path.string().c_str(),
                width,
                height,
                4,
                flipped,
                width * 4
            );

            if (!success) {
                log::warn("Failed to save screenshot with stb_image_write to {}", path.string());
                delete[] buffer;
                delete[] flipped;
                return false;
            } else {
                log::info("Screenshot saved to {}", path.string());
            }

            delete[] buffer;
            delete[] flipped;
            return true;
        }

        // inline bool startRecording() {}
        // inline bool stopRecording() {}
    };
}
