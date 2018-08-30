#include "renderer/RenderContext.hpp"
#include "renderer/GameTexture.hpp"
#include "stb_image/stb_image.h"
#include <algorithm>
#include <cmath>
#ifdef __ANDROID__
#include <android/asset_manager.h>

extern AAssetManager *g_androidAssetMgr;
#endif

extern RenderContext  g_renderContext;

GameTexture::GameTexture(const char *filename)
{
    VkFormatProperties fp = {};
    vkGetPhysicalDeviceFormatProperties(g_renderContext.Device().physical, VK_FORMAT_R8G8B8_UNORM, &fp);

    bool canBlitLinear  = (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) != 0;
    bool canBlitOptimal = (fp.linearTilingFeatures  & VK_FORMAT_FEATURE_BLIT_DST_BIT) != 0;

#ifdef __ANDROID__
    AAsset *asset = AAssetManager_open(g_androidAssetMgr, filename, AASSET_MODE_STREAMING);
    size_t texSize = AAsset_getLength64(asset);
    char *texBuffer = new char[texSize];
    AAsset_read(asset, texBuffer, texSize);
    AAsset_close(asset);

    // force rgba if the device can't blit to rgb format (required by mipmapping)
    if (canBlitLinear || canBlitOptimal)
    {
        m_textureData = stbi_load_from_memory((const unsigned char*)texBuffer, texSize, &m_width, &m_height, &m_components, STBI_default);
    }
    else
    {
        m_textureData = stbi_load_from_memory((const unsigned char*)texBuffer, texSize, &m_width, &m_height, &m_components, STBI_rgb_alpha);
        m_components = 4;
    }

    delete[] texBuffer;
#else
    // force rgba if the device can't blit to rgb format (required by mipmapping)
    if (canBlitLinear || canBlitOptimal)
    {
        m_textureData = stbi_load(filename, &m_width, &m_height, &m_components, STBI_default);
    }
    else
    {
        m_textureData = stbi_load(filename, &m_width, &m_height, &m_components, STBI_rgb_alpha);
        m_components = 4;
    }
#endif
}

GameTexture::~GameTexture()
{
    if (m_textureData != nullptr)
        stbi_image_free(m_textureData);

    vk::releaseTexture(g_renderContext.Device(), m_vkTexture);
}

bool GameTexture::Load(bool filtering)
{
    // texture already loaded or doesn't exist
    if (!m_textureData)
        return false;

    if (!filtering)
    {
        m_vkTexture.minFilter = VK_FILTER_NEAREST;
        m_vkTexture.magFilter = VK_FILTER_NEAREST;
    }

    m_vkTexture.format = m_components == 3 ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
    // calculate number of mipmaps to generate for given texture dimensions
    m_vkTexture.mipLevels = (uint32_t)std::floor(std::log2(std::max(m_width, m_height))) + 1;

    vk::createTexture(g_renderContext.Device(), &m_vkTexture, m_textureData, m_width, m_height);

    stbi_image_free(m_textureData);
    m_textureData = nullptr;

    return true;
}
