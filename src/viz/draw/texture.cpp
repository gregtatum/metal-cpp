#include "./texture.h"

namespace viz {

size_t
GetBytesPerRow(PixelFormat format, uint32_t width)
{
  switch (format) {
    case PixelFormat::BGRA8Unorm:
      return 4 * width;
    default:
      throw ErrorMessage("TODO - The switch case needs this format added.");
  }
}

Texture2D::Texture2D(Texture2DInitializer&& initializer)
{
  auto& [device,
         format,
         width,
         height,
         mipmapped,
         textureType,
         mipmapLevelCount,
         sampleCount,
         arrayLength,
         resourceOptions,
         cpuCacheMode,
         storageMode,
         usage] = initializer;

  descriptor =
    TextureDescriptor::Texture2DDescriptor(format, width, height, mipmapped);
  auto bytesPerRow = GetBytesPerRow(format, width);

  if (textureType)
    descriptor.SetTextureType(textureType.value());
  if (mipmapLevelCount)
    descriptor.SetMipmapLevelCount(mipmapLevelCount.value());
  if (sampleCount)
    descriptor.SetSampleCount(sampleCount.value());
  if (arrayLength)
    descriptor.SetArrayLength(arrayLength.value());
  if (resourceOptions)
    descriptor.SetResourceOptions(resourceOptions.value());
  if (cpuCacheMode)
    descriptor.SetCpuCacheMode(cpuCacheMode.value());
  if (storageMode)
    descriptor.SetStorageMode(storageMode.value());
  if (usage)
    descriptor.SetUsage(usage.value());

  texture = device.NewTexture(descriptor);
}

template<typename Format, typename Fn>
void
Texture2D::SetData(Fn fn)
{
  // Determine the size of things.
  uint32_t width = texture.GetWidth();
  uint32_t height = texture.GetHeight();
  auto size = width * height;
  auto bytes = sizeof(Format) * size;
  auto bytesPerRow = sizeof(Format) * width;

  if (texture.GetMipmapLevelCount() != 0) {
    throw new ErrorMessage(
      "Texture2D::SetData does not support mipmapped data.");
  }

  // First, allocate the data in the heap. The GPU must copy this
  // into its own memory space.
  std::vector<Format> data;
  data.reserve(size);

  // Run the lambda to set the data, all with the CPU.
  for (uint32_t i = 0; i < size; i++) {
    uint32_t x = i / width;
    uint32_t y = i % width;
    data.push_back(fn(x, y));
  }

  // The texture will read in the data, and replace its internal memory.
  Region region{ 0, 0, width, height };
  texture.Replace(region, 0, data.data(), bytesPerRow);
}

} // namespace viz
