#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "Utils.hpp"

namespace dashcam {

Utils* Utils::shared() {
    static Utils instance;
    return &instance;
}

// void Utils::renderSceneToTexture() {
//     CCSize winSize = CCDirector::sharedDirector()->getWinSize();

//     if (rt) {
//         rt->removeFromParentAndCleanup(true);
//         rt = nullptr;
//     }

//     rt = CCRenderTexture::create(winSize.width, winSize.height);
//     rt->setPosition(ccp(winSize.width / 2, winSize.height / 2));
//     rt->setAnchorPoint(ccp(0.5f, 0.5f));
//     CCDirector::sharedDirector()->getRunningScene()->addChild(rt, INT_MAX);

//     CCDirector::sharedDirector()->getRunningScene()->setVisible(false);
//     rt->begin();
//     CCDirector::sharedDirector()->getRunningScene()->visit();
//     rt->end();
//     CCDirector::sharedDirector()->getRunningScene()->setVisible(true);

//     rt->getSprite()->setFlipY(true);
// }

// void Utils::applyShader(ShaderType shader) {
//     if (!rt) {
//         renderSceneToTexture();
//     }

//     switch (shader) {
//         case Invert: {
//             CCGLProgram* program = new CCGLProgram();
//             program->initWithVertexShaderFilename("invert.vsh", "invert.fsh");
//             program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
//             program->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
//             program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
//             program->link();
//             program->updateUniforms();
//             rt->getSprite()->setShaderProgram(program);
//             break;
//         }

//         case Pixelated:
//             // Add pixelation shader setup here
//             break;

//         case Blur:
//             // Add blur shader setup here
//             break;

//         default:
//             break;
//     }
// }

bool Utils::screenshot() {
    log::info("Screenshot event triggered");

    // auto shaderSetting = Mod::get()->getSettingValue<std::string>("shader");
    // if (shaderSetting == "Invert") {
    //     applyShader(ShaderType::Invert);
    // } else if (shaderSetting == "Pixelated") {
    //     applyShader(ShaderType::Pixelated);
    // } else if (shaderSetting == "Blur") {
    //     applyShader(ShaderType::Blur);
    // }

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
        std::vector<unsigned char> flippedRGB(width * height * 3);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int srcIndex = ((height - 1 - y) * width + x) * 4;
                int dstIndex = (y * width + x) * 3;

                flippedRGB[dstIndex + 0] = data[srcIndex + 0]; // R
                flippedRGB[dstIndex + 1] = data[srcIndex + 1]; // G
                flippedRGB[dstIndex + 2] = data[srcIndex + 2]; // B
            }
        }

        int success = stbi_write_png(
            path.string().c_str(),
            width,
            height,
            3,
            flippedRGB.data(),
            width * 3
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

} // namespace dashcam
