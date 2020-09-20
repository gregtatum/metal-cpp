#include "viz/metal.h"

namespace viz {

// Texture format representations
struct BGRA8Unorm
{
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};

struct Texture2DInitializer
{
  Device& device;
  PixelFormat pixelFormat;
  uint32_t width;
  uint32_t height;
  bool mipmapped = false;

  std::optional<TextureType> textureType;
  std::optional<uint32_t> mipmapLevelCount;
  std::optional<uint32_t> sampleCount;
  std::optional<uint32_t> arrayLength;
  std::optional<ResourceOptions> resourceOptions;
  std::optional<CpuCacheMode> cpuCacheMode;
  std::optional<StorageMode> storageMode;
  std::optional<TextureUsage> usage;
};

/**
 * Initialize and draw to a texture.
 */
class Texture2D
{
public:
  explicit Texture2D(Texture2DInitializer&& initializer);

  // Set the data with a user-provided callback function.
  // The function signature is:
  //
  // (uint32_t x, uint32_t y) -> Format
  template<typename Format, typename Fn>
  void SetData(Fn fn);

  TextureDescriptor descriptor;
  Texture texture;
};

} // namespace viz
