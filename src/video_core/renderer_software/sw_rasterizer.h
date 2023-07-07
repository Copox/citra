// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <span>

#include "video_core/rasterizer_interface.h"
#include "video_core/regs_texturing.h"
#include "video_core/renderer_software/sw_clipper.h"
#include "video_core/renderer_software/sw_framebuffer.h"

namespace Pica::Shader {
struct OutputVertex;
}

namespace Pica {
struct State;
struct Regs;
} // namespace Pica

namespace SwRenderer {

struct Vertex;

class RasterizerSoftware : public VideoCore::RasterizerInterface {
public:
    explicit RasterizerSoftware(Memory::MemorySystem& memory);

    void AddTriangle(const Pica::Shader::OutputVertex& v0, const Pica::Shader::OutputVertex& v1,
                     const Pica::Shader::OutputVertex& v2) override;
    void DrawTriangles() override {}
    void NotifyPicaRegisterChanged(u32 id) override {}
    void FlushAll() override {}
    void FlushRegion(PAddr addr, u32 size) override {}
    void InvalidateRegion(PAddr addr, u32 size) override {}
    void FlushAndInvalidateRegion(PAddr addr, u32 size) override {}
    void ClearAll(bool flush) override {}

private:
    /// Computes the screen coordinates of the provided vertex.
    void MakeScreenCoords(Vertex& vtx);

    /// Processes the triangle defined by the provided vertices.
    void ProcessTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                         bool reversed = false);

    /// Returns the texture color of the currently processed pixel.
    std::array<Common::Vec4<u8>, 4> TextureColor(
        std::span<const Common::Vec2<f24>, 3> uv,
        std::span<const Pica::TexturingRegs::FullTextureConfig, 3> textures, f24 tc0_w) const;

    /// Returns the final pixel color with blending or logic ops applied.
    Common::Vec4<u8> PixelColor(u16 x, u16 y, Common::Vec4<u8>& combiner_output) const;

    /// Emulates the TEV configuration and returns the combiner output.
    void WriteTevConfig(std::span<const Common::Vec4<u8>, 4> texture_color,
                        std::span<const Pica::TexturingRegs::TevStageConfig, 6> tev_stages,
                        Common::Vec4<u8> primary_color, Common::Vec4<u8> primary_fragment_color,
                        Common::Vec4<u8> secondary_fragment_color);

    /// Blends fog to the combiner output if enabled.
    void WriteFog(Common::Vec4<u8>& combiner_output, float depth) const;

    /// Performs the alpha test. Returns false if the test failed.
    bool DoAlphaTest(u8 alpha) const;

    /// Performs the depth stencil test. Returns false if the test failed.
    bool DoDepthStencilTest(u16 x, u16 y, float depth) const;

private:
    Memory::MemorySystem& memory;
    Pica::State& state;
    const Pica::Regs& regs;
    Framebuffer fb;
    // Kirby Blowout Blast relies on the combiner output of a previous draw
    // in order to render the sky correctly.
    Common::Vec4<u8> combiner_output{};
};

} // namespace SwRenderer