/* This is a C++ immediate mode graphics library.

Setup:
Define the 'NMD_GRAPHICS_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_GRAPHICS_IMPLEMENTATION
#include "nmd_graphics.hpp"

Low level overview:
The nmd::Context(acessible by nmd::GetContext()) global variable holds the state of the entire library, it
contains a nmd::DrawList variable which holds the vertex, index and commands buffers. Each command buffer 
translate to a call to a rendering's API draw function. The nmd::DrawList class has methods to draw basic 
geometry shapes(e.g. circles, rectangles and lines).

Supported rendering APIs: Direct3D 9(D3D9), Direct3D 11(D3D11).
To use a specific rendering api define the macro 'NMD_GRAPHICS_{RENDERING API}' before including "nmd_graphics.hpp".

Usage:
 - General:
    - Call API functions between nmd::Begin() and nmd::End()
 - D3D9:
    - Call nmd::D3D9SetDevice() and nmd::D3D9Resize() on initialization.
    - Call nmd::D3D9Render() after nmd::End().

 - D3D11:
    - Call nmd::D3D11SetDeviceContext() on initialization.
    - Call nmd::D3D11Render() after nmd::End()

Default fonts:
The 'Karla' true type font in included by default. Define the 'NMD_GRAPHICS_DISABLE_DEFAULT_FONT' macro to remove the
font at compile time. By doing so at least 15KB of code & data will be saved.

TODO:
 - Add support for textures in Direct3D 11.
 - Add AddText() method to DrawList.
 - Add support for the remaining rendering APIs: Direct3D 12, OpenGL and Vulkan.

Credits:
 - imgui - https://github.com/ocornut/imgui
 - stb_truetype - https://github.com/nothings/stb/blob/master/stb_truetype.h
*/

#ifndef NMD_GRAPHICS_H
#define NMD_GRAPHICS_H

// Common dependencies
#include <inttypes.h>
#include <stddef.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef NMD_GRAPHICS_D3D9
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#endif

#ifdef NMD_GRAPHICS_D3D11
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#endif

namespace nmd
{
//#ifdef _WIN32
//    void Win32Init(HWND hwnd);
//    LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//#endif

#ifdef NMD_GRAPHICS_D3D9
    void D3D9SetDevice(LPDIRECT3DDEVICE9 pDevice);
    void D3D9Render();
    void D3D9Resize(int width, int height);
#endif

#ifdef NMD_GRAPHICS_D3D11
    void D3D11SetDeviceContext(ID3D11DeviceContext* pDeviceContext);
    void D3D11Render();
#endif
    struct Color
    {
        static const Color Black;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Orange;
        static const Color Amber;
        static const Color AndroidGreen;
        static const Color Azure;
        static const Color Bronze;
        static const Color Corn;
        static const Color Emerald;
        static const Color LapisLazuli;
        static const Color Lava;

        union
        {
            uint32_t color;
            struct { uint8_t r, g, b, a; };
        };

        Color(uint32_t color)
            : color(color) {}

        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            : r(r), g(g), b(b), a(a) {}

        Color() : Color(0) {}
    };

    struct Vec2
    {
        float x, y;

        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float xy) : x(xy), y(xy) {}
        Vec2(float x, float y) : x(x), y(y) {}

        Vec2 operator+(const Vec2& other) const;
        Vec2 operator-(const Vec2& other) const;
        Vec2 operator*(const Vec2& other) const;

        static Vec2 Clamp(const Vec2& x, const Vec2& low, const Vec2& high);
        static Vec2 Min(const Vec2& lhs, const Vec2& rhs);
        static Vec2 Max(const Vec2& lhs, const Vec2& rhs);
    };

    struct Vec3
    {
        float x, y, z;

        Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vec3 operator+(const Vec3& other) { return Vec3(this->x + other.x, this->y + other.y, this->z + other.z); }
        void operator+=(const Vec3& other) { this->x += other.x; this->y += other.y; this->z += other.z; }
    };

    struct Vec4
    {
        union
        {
            struct { float x, y, z, w; };
            struct { float left, top, right, bottom; };
            struct { Vec2 pos, size; };
        };

        Vec4() : pos(), size() {}
        Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        Vec4(const Vec2& pos, const Vec2& size) : pos(pos), size(size) {}

        Vec4 operator+(const Vec4& other) { return Vec4(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w); }
        void operator+=(const Vec4& other) { this->x += other.x, this->y + other.y, this->z + other.z, this->w + other.w; }
    };

    bool IsPointInRect(const Vec4& rect, const Vec2& p);

    enum CORNER_FLAGS
    {
        CORNER_FLAGS_NONE         = (1 << 0),
        CORNER_FLAGS_TOP_LEFT     = (1 << 1),
        CORNER_FLAGS_TOP_RIGHT    = (1 << 2),
        CORNER_FLAGS_BOTTOM_LEFT  = (1 << 3),
        CORNER_FLAGS_BOTTOM_RIGHT = (1 << 4),
        CORNER_FLAGS_ALL          = (1 << 5) - 1,
        CORNER_FLAGS_TOP          = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_TOP_RIGHT,
        CORNER_FLAGS_BOTTOM       = CORNER_FLAGS_BOTTOM_LEFT | CORNER_FLAGS_BOTTOM_RIGHT,
        CORNER_FLAGS_LEFT         = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_BOTTOM_LEFT,
        CORNER_FLAGS_RIGHT        = CORNER_FLAGS_TOP_RIGHT | CORNER_FLAGS_BOTTOM_RIGHT
    };

    typedef uint16_t IndexType;
    typedef void* TextureId;

    struct DrawCommand
    {
        IndexType numVertices; //numVertices uses IndexType because the number of vertices is always less or equal the number of indices.
        IndexType numIndices;
        TextureId userTextureId;

        DrawCommand(IndexType numVertices, IndexType numIndices, TextureId userTextureId)
            : numVertices(numVertices), numIndices(numIndices), userTextureId(userTextureId) {}
    };

    struct Vertex
    {
        Vec2 pos;
        Color color;
        Vec2 uv;

        Vertex() : pos(), color(), uv() {}
        Vertex(const Vec2& pos, const Color& color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}
    };

    class DrawList
    {
    public:
        DrawList();
        DrawList(const DrawList&) = delete;

        Vec2 cachedCircleVertices12[12];
        uint8_t cachedCircleSegmentCounts64[64];
        float curveTessellationTolerance;
        void CalculateCircleSegments(float maxError);

        std::vector<Vec2> path;

        std::vector<Vertex> vertices;
        std::vector<IndexType> indices;

        std::vector<DrawCommand> drawCommands;
        void PushRemainingDrawCommands();
        void PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId);

        void AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness = 1.0f);

        void AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL, float thickness = 1.0f);
        void AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding = 0.0f, uint32_t cornerFlags = CORNER_FLAGS_ALL);
        void AddRectFilledMultiColor(const Vec2& p1, const Vec2& p2, Color colorUpperLeft, Color colorUpperRight, Color colorBottomRight, Color colorBottomLeft);

        void AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness = 1.0f);
        void AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color);

        void AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness = 1.0f);
        void AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color);

        //Set numSegments to zero(0) if you want the function to automatically determine the number of segmnts.
        void AddCircle(const Vec2& center, float radius, Color color, size_t numSegments = 12, float thickness = 1.0f);
        void AddCircleFilled(const Vec2& center, float radius, Color color, size_t numSegments = 12);

        void AddNgon(const Vec2& center, float radius, Color color, size_t numSegments, float thickness = 1.0f);
        void AddNgonFilled(const Vec2& center, float radius, Color color, size_t numSegments);

        void AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed = false, float thickness = 1.0f);
        void AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color);

        void AddBezierCurve(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness = 1.0f, size_t numSegments = 0);

        void AddText(const Vec2& pos, Color color, const char* text, size_t textLength);
        void AddText(const void* font, float fontSize, const Vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth = 0.0f);

        void AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);
        void AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 0), const Vec2& uv3 = Vec2(1, 1), const Vec2& uv4 = Vec2(0, 1), Color color = Color::White);
        void AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags = CORNER_FLAGS_ALL, const Vec2& uv1 = Vec2(0, 0), const Vec2& uv2 = Vec2(1, 1), Color color = Color::White);

        //Path API
        inline void PathLineTo(const Vec2& pos) { path.push_back(pos); }
        void PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t rounding_corners);

        //startAtCenter places the first vertex at the center, this can be used to create a pie chart when using PathFillConvex().
        //This functions uses twelve(12) chached vertices initialized during startup, it should be faster than PathArcTo.
        void PathArcToCached(const Vec2& center, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter = false);
        void PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, size_t numSegments = 10, bool startAtCenter = false);

        inline void PathStroke(Color color, bool closed, float thickness = 1.0f) { AddPolyline(path.data(), path.size(), color, closed, thickness); path.clear(); }
        inline void PathFillConvex(Color color) { AddConvexPolyFilled(path.data(), path.size(), color); path.clear(); }

        void PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments);

        void PrimRect(const Vec2& p1, const Vec2& p2, Color color);
        void PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color);
        void PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color);
    };

    struct Context
    {
        DrawList drawList;
    };

    Context& GetContext();

    // Begin() clears all of the vertices, indices and command buffers. Call this function to start a new scene.
    void Begin();

    // End() calculates the remaining command buffers. Call this function before rendering.
    void End();

} // namespace nmd

#endif // NMD_GRAPHICS_H


#ifdef NMD_GRAPHICS_IMPLEMENTATION

#define NMD_PI  3.141592653f
#define NMD_2PI 6.283185306f

#define NMD_CLAMP(x, low, high) ((x < low) ? low : (x > high) ? high : x)
#define NMD_MIN(a, b) (a < b ? a : b)
#define NMD_MAX(a, b) (a > b ? a : b)

#define CIRCLE_AUTO_SEGMENT_MIN 12
#define CIRCLE_AUTO_SEGMENT_MAX 512
#define CIRCLE_AUTO_SEGMENT_CALC(radius, maxError) NMD_CLAMP(NMD_2PI / acosf((radius - maxError) / radius), CIRCLE_AUTO_SEGMENT_MIN, CIRCLE_AUTO_SEGMENT_MAX)

namespace nmd
{
	Vec2 Vec2::operator+(const Vec2& other) const { return Vec2(this->x + other.x, this->y + other.x); }
	Vec2 Vec2::operator-(const Vec2& other) const { return Vec2(this->x - other.x, this->y - other.x); }
	Vec2 Vec2::operator*(const Vec2& other) const { return Vec2(this->x * other.x, this->y * other.x); }

	Vec2 Vec2::Clamp(const Vec2& x, const Vec2& low, const Vec2& high) { return Vec2(NMD_CLAMP(x.x, low.x, high.x), NMD_CLAMP(x.y, low.y, high.y)); }
	Vec2 Vec2::Min(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MIN(lhs.x, rhs.x), NMD_MIN(lhs.y, rhs.y)); }
	Vec2 Vec2::Max(const Vec2& lhs, const Vec2& rhs) { return Vec2(NMD_MAX(lhs.x, rhs.x), NMD_MAX(lhs.y, rhs.y)); }

	bool IsPointInRect(const Vec4& rect, const Vec2& p) { return p.x >= rect.pos.x && p.x <= rect.pos.x + rect.size.x && p.y >= rect.pos.y && p.y <= rect.pos.y + rect.size.y; }

#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_FONT
	extern const uint8_t karla_ttf_regular[14824];
#endif // NMD_GRAPHICS_DISABLE_DEFAULT_FONT
} // namespace nmd


#include "stb_truetype.h"

namespace nmd
{
    void PathBezierToCasteljau(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, size_t level)
    {
        const float dx = x4 - x1;
        const float dy = y4 - y1;
        float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
        float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
        d2 = (d2 >= 0) ? d2 : -d2;
        d3 = (d3 >= 0) ? d3 : -d3;
        if ((d2 + d3) * (d2 + d3) < GetContext().drawList.curveTessellationTolerance * (dx * dx + dy * dy))
            GetContext().drawList.path.emplace_back(x4, y4);
        else if (level < 10)
        {
            const float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            const float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            const float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
            const float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            const float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
            const float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
            PathBezierToCasteljau(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
            PathBezierToCasteljau(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
        }
    }

    Vec2 BezierCalc(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float t)
    {
        const float u = 1.0f - t;
        const float w1 = u * u * u;
        const float w2 = 3 * u * u * t;
        const float w3 = 3 * u * t * t;
        const float w4 = t * t * t;
        return Vec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
    }

    // Distribute UV over (a, b) rectangle
    void ShadeVertsLinearUV(size_t startVertexIndex, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, bool clamp)
    {
        const Vec2 size = p2 - p1;
        const Vec2 uv_size = uv2 - uv1;
        const Vec2 scale = Vec2(size.x != 0.0f ? (uv_size.x / size.x) : 0.0f, size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

        Vertex* const startVertex = GetContext().drawList.vertices.data() + startVertexIndex;
        const Vertex* const endVertex = &GetContext().drawList.vertices.back();
        if (clamp)
        {
            const Vec2 min = Vec2::Min(uv1, uv2), max = Vec2::Max(uv1, uv2);
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = Vec2::Clamp(uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale), min, max);
        }
        else
        {
            for (Vertex* vertex = startVertex; vertex < endVertex; ++vertex)
                vertex->uv = uv1 + ((Vec2(vertex->pos.x, vertex->pos.y) - p1) * scale);
        }
    }

    DrawList::DrawList()
        : curveTessellationTolerance(1.25f)
    {
        for (size_t i = 0; i < 12; i++)
        {
            //const float angle = (i / 12.0f) * GUI_2PI;
            const float angle = (i / 6.0f) * NMD_PI; // Simplified version of the line above.
            cachedCircleVertices12[i] = Vec2(cosf(angle), sinf(angle));
        }

        CalculateCircleSegments(1.6f);
    }

    void DrawList::CalculateCircleSegments(float maxError)
    {
        for (size_t i = 0; i < 64; i++)
        {
            const uint8_t segment_count = static_cast<uint8_t>(CIRCLE_AUTO_SEGMENT_CALC(i + 1.0f, maxError));
            cachedCircleSegmentCounts64[i] = NMD_MIN(segment_count, 255);
        }
    }

    void DrawList::PushRemainingDrawCommands()
    {
        size_t numAccountedVertices = 0, numAccountedIndices = 0;
        for (auto& drawCommand : drawCommands)
            numAccountedVertices += drawCommand.numVertices, numAccountedIndices += drawCommand.numIndices;

        size_t numUnaccountedIndices = indices.size() - numAccountedIndices;

        while (numUnaccountedIndices > 0)
        {
            if (numUnaccountedIndices <= (2 << (8 * sizeof(IndexType) - 1)))
            {
                drawCommands.emplace_back(static_cast<IndexType>(vertices.size() - numAccountedVertices), static_cast<IndexType>(numUnaccountedIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices = 0;
                return;
            }
            else
            {
                size_t numIndices = (2 << (8 * sizeof(IndexType) - 1)) - 1;
                IndexType lastIndex = indices[numIndices - 1];

                bool isLastIndexReferenced = false;
                do
                {
                    for (size_t i = numIndices; i < numUnaccountedIndices; i++)
                    {
                        if (indices[i] == lastIndex)
                        {
                            isLastIndexReferenced = true;
                            numIndices -= 3;
                            lastIndex = indices[numIndices - 1];
                            break;
                        }
                    }
                } while (isLastIndexReferenced);

                drawCommands.emplace_back(static_cast<IndexType>(lastIndex + 1), static_cast<IndexType>(numIndices), static_cast<TextureId>(NULL));
                numUnaccountedIndices -= numIndices;
            }
        }
    }

    void DrawList::PushTextureDrawCommand(size_t numVertices, size_t numIndices, TextureId userTextureId)
    {
        if (!drawCommands.empty() && drawCommands.back().userTextureId == userTextureId)
            drawCommands.back().numVertices += static_cast<IndexType>(numVertices), drawCommands.back().numIndices += static_cast<IndexType>(numIndices);
        else
            drawCommands.emplace_back(static_cast<IndexType>(numVertices), static_cast<IndexType>(numIndices), userTextureId);
    }

    void DrawList::AddRect(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags, float thickness)
    {
        if (!color.a || thickness == 0.0f)
            return;
        
        PathRect(p1 + Vec2(0.5f, 0.5f), p2 - Vec2(0.5f, 0.5f), rounding, cornerFlags);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddRectFilled(const Vec2& p1, const Vec2& p2, Color color, float rounding, uint32_t cornerFlags)
    {
        if (!color.a)
            return;

        if (rounding > 0.0f)
        {
            PathRect(p1, p2, rounding, cornerFlags);
            PathFillConvex(color);
        }
        else
            PrimRect(p1, p2, color);
    }

    void DrawList::AddRectFilledMultiColor(const Vec2& p1, const Vec2& p2, Color colorUpperLeft, Color colorUpperRight, Color colorBottomRight, Color colorBottomLeft)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, colorUpperLeft, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), colorUpperRight, Vec2());
        vertices.emplace_back(p2, colorBottomRight, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), colorBottomLeft, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::AddQuad(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddQuadFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathFillConvex(color);
    }

    void DrawList::AddTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color)
    {
        if (!color.a)
            return;

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(p3, color, Vec2());
        for (size_t i = 0; i < 3; i++)
            indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddCircle(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments == 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius - 0.5f, 0, 12);
        else
            PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * (static_cast<float>(numSegments - 1) / static_cast<float>(numSegments)), numSegments - 1);

        PathStroke(color, true, thickness);
    }

    void DrawList::AddCircleFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || radius <= 0.0f)
            return;

        if (numSegments <= 0)
            numSegments = (static_cast<size_t>(radius) - 1 < 64) ? cachedCircleSegmentCounts64[static_cast<size_t>(radius) - 1] : CIRCLE_AUTO_SEGMENT_CALC(radius, 1.6f);
        else
            numSegments = NMD_CLAMP(numSegments, 3, CIRCLE_AUTO_SEGMENT_MAX);

        if (numSegments == 12)
            PathArcToCached(center, radius, 0, 12);
        else
            PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);

        PathFillConvex(color);
    }

    void DrawList::AddNgon(const Vec2& center, float radius, Color color, size_t numSegments, float thickness)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius - 0.5f, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathStroke(color, true, thickness);
    }

    void DrawList::AddNgonFilled(const Vec2& center, float radius, Color color, size_t numSegments)
    {
        if (!color.a || numSegments < 3)
            return;

        //Remove one(1) from numSegment because it's a closed shape.
        PathArcTo(center, radius, 0.0f, NMD_2PI * ((static_cast<float>(numSegments) - 1.0f) / static_cast<float>(numSegments)), numSegments - 1);
        PathFillConvex(color);
    }

    void DrawList::PathRect(const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags)
    {
        if (rounding <= 0.0f || cornerFlags == 0)
        {
            PathLineTo(p1);
            PathLineTo(Vec2(p2.x, p1.y));
            PathLineTo(p2);
            PathLineTo(Vec2(p1.x, p2.y));
        }
        else
        {
            const float roundingTopLeft = (cornerFlags & CORNER_FLAGS_TOP_LEFT) ? rounding : 0.0f;
            const float roundingTopRight = (cornerFlags & CORNER_FLAGS_TOP_RIGHT) ? rounding : 0.0f;
            const float roundingBottomRight = (cornerFlags & CORNER_FLAGS_BOTTOM_RIGHT) ? rounding : 0.0f;
            const float roundingBottomLeft = (cornerFlags & CORNER_FLAGS_BOTTOM_LEFT) ? rounding : 0.0f;
            PathArcToCached(Vec2(p1.x + roundingTopLeft, p1.y + roundingTopLeft), roundingTopLeft, 6, 9);
            PathArcToCached(Vec2(p2.x - roundingTopRight, p1.y + roundingTopRight), roundingTopRight, 9, 12);
            PathArcToCached(Vec2(p2.x - roundingBottomRight, p2.y - roundingBottomRight), roundingBottomRight, 0, 3);
            PathArcToCached(Vec2(p1.x + roundingBottomLeft, p2.y - roundingBottomLeft), roundingBottomLeft, 3, 6);
        }
    }

    void DrawList::PathArcTo(const Vec2& center, float radius, float startAngle, float endAngle, size_t numSegments, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t i = 0; i <= numSegments; i++)
        {
            const float angle = startAngle + (static_cast<float>(i) / static_cast<float>(numSegments)) * (endAngle - startAngle);
            path.emplace_back(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        }
    }

    void DrawList::PathArcToCached(const Vec2& center, float radius, size_t startAngleOf12, size_t endAngleOf12, bool startAtCenter)
    {
        if (startAtCenter)
            path.push_back(center);

        for (size_t angle = startAngleOf12; angle <= endAngleOf12; angle++)
        {
            const Vec2& point = cachedCircleVertices12[angle % 12];
            path.emplace_back(center.x + point.x * radius, center.y + point.y * radius);
        }
    }

    void DrawList::PathBezierCurveTo(const Vec2& p2, const Vec2& p3, const Vec2& p4, size_t numSegments)
    {
        const Vec2& p1 = path.back();
        if (numSegments == 0)
            PathBezierToCasteljau(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, 0);
        else
        {
            const float tStep = 1.0f / static_cast<float>(numSegments);
            for (size_t iStep = 1; iStep <= numSegments; iStep++)
                path.push_back(BezierCalc(p1, p2, p3, p4, tStep * iStep));
        }
    }

    void DrawList::PrimRect(const Vec2& p1, const Vec2& p2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, Vec2());
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2());
        vertices.emplace_back(p2, color, Vec2());
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2());

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimRectUV(const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(Vec2(p2.x, p1.y), color, Vec2(uv2.x, uv1.y));
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(Vec2(p1.x, p2.y), color, Vec2(uv1.x, uv2.y));

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::PrimQuadUV(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        const IndexType nextIndex = static_cast<IndexType>(vertices.size());

        vertices.emplace_back(p1, color, uv1);
        vertices.emplace_back(p2, color, uv2);
        vertices.emplace_back(p3, color, uv3);
        vertices.emplace_back(p4, color, uv4);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 1);
        indices.push_back(nextIndex + 2);

        indices.push_back(nextIndex + 0);
        indices.push_back(nextIndex + 2);
        indices.push_back(nextIndex + 3);
    }

    void DrawList::AddLine(const Vec2& p1, const Vec2& p2, Color color, float thickness)
    {
        if (!color.a)
            return;

        PathLineTo(p1 + Vec2(0.5f, 0.5f));
        PathLineTo(p2 + Vec2(0.5f, 0.5f));
        PathStroke(color, false, thickness);
    }

    void DrawList::AddPolyline(const Vec2* points, size_t numPoints, Color color, bool closed, float thickness)
    {
        if (numPoints < 2)
            return;

        IndexType nextIndex = static_cast<IndexType>(vertices.size());
        const float halfThickness = (thickness * 0.5f);
        for (size_t i = 0; i < (closed ? numPoints : numPoints - 1); i++, nextIndex += 4)
        {
            const Vec2& p1_tmp = points[i], & p2_tmp = points[(i + 1) == numPoints ? 0 : i + 1];
            const float dx = p2_tmp.x - p1_tmp.x;
            const float dy = p2_tmp.y - p1_tmp.y;

            //If we didn't swap the points in the cases the triangles would be drawn in the counter clockwise direction, which can cause problems in some rendering APIs.
            const bool swapPoints = (dx < 0.0f || dy < 0.0f) || (dx > 0.0f && dy > 0.0f);
            const Vec2& p1 = swapPoints ? p2_tmp : p1_tmp, & p2 = swapPoints ? p1_tmp : p2_tmp;

            if (dy == 0) // Horizontal line
            {
                int factor = dx > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y - halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness * factor, p2.y + halfThickness), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness * factor, p1.y + halfThickness), color, Vec2());
            }
            else if (dx == 0) // Vertical line
            {
                int factor = dy > 0.0f ? 1 : -1;
                vertices.emplace_back(Vec2(p1.x + halfThickness, p1.y - halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - halfThickness, p2.y + halfThickness * factor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x - halfThickness, p1.y - halfThickness * factor), color, Vec2());
            }
            else // Inclined line
            {
                const float lineWidth = sqrtf(dx * dx + dy * dy);

                const float cosine = dx / lineWidth;
                const float sine = dy / lineWidth;

                const float xFactor = cosine * halfThickness;
                const float yFactor = sine * halfThickness;

                vertices.emplace_back(Vec2(p1.x - yFactor, p1.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x - yFactor, p2.y + xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p2.x + yFactor, p2.y - xFactor), color, Vec2());
                vertices.emplace_back(Vec2(p1.x + yFactor, p1.y - xFactor), color, Vec2());
            }

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 1);
            indices.push_back(nextIndex + 2);

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 2);
            indices.push_back(nextIndex + 3);
        }
    }

    void DrawList::AddConvexPolyFilled(const Vec2* points, size_t numPoints, Color color)
    {
        if (numPoints < 3)
            return;

        const IndexType nextIndex = static_cast<IndexType>(vertices.size());
        for (size_t i = 0; i < numPoints; i++)
            vertices.emplace_back(points[i], color, Vec2());

        for (size_t i = 2; i < numPoints; i++)
            indices.push_back(nextIndex), indices.push_back(nextIndex + static_cast<IndexType>(i - 1)), indices.push_back(nextIndex + static_cast<IndexType>(i));
    }

    void DrawList::AddBezierCurve(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Color color, float thickness, size_t numSegments)
    {
        if (!color.a)
            return;

        PathLineTo(p1);
        PathBezierCurveTo(p2, p3, p4, numSegments);
        PathStroke(color, false, thickness);
    }

    void DrawList::AddText(const Vec2& pos, Color color, const char* text, size_t textLength)
    {
        AddText(NULL, 0.0f, pos, color, text, textLength);
    }
    
    void DrawList::AddText(const void* font, float fontSize, const Vec2& pos, Color color, const char* text, size_t textLength, float wrapWidth)
    {
        if (!color.a)
            return;
        
        const char* const textEnd = text + textLength;
        
        float x = pos.x;
        float y = pos.y;

        while (text < textEnd)
        {
            //const Glyph* glyph = font->FindGlyph(*text);
        
            //stbtt_aligned_quad q;
            //stbtt_GetBakedQuad(bdata, 512, 512, *text - 32, &x, &y, &q, 0);
            //
            //const size_t nextIndex = vertices.size();
            //vertices.emplace_back(Vec2(glyph->x0, glyph->y0), color, Vec2(glyph->u0, glyph->v0));
            //vertices.emplace_back(Vec2(glyph->x1, glyph->y0), color, Vec2(glyph->u1, glyph->v0));
            //vertices.emplace_back(Vec2(glyph->x1, glyph->y1), color, Vec2(glyph->u1, glyph->v1));
            //vertices.emplace_back(Vec2(glyph->x0, glyph->y1), color, Vec2(glyph->u0, glyph->v1));
        
            const IndexType nextIndex = static_cast<IndexType>(vertices.size());

            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 1);
            indices.push_back(nextIndex + 2);
        
            indices.push_back(nextIndex + 0);
            indices.push_back(nextIndex + 2);
            indices.push_back(nextIndex + 3);
        
            text++;
        }
    }

    void DrawList::AddImage(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimRectUV(p1, p2, uv1, uv2, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageQuad(TextureId userTextureId, const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Vec2& uv1, const Vec2& uv2, const Vec2& uv3, const Vec2& uv4, Color color)
    {
        if (!color.a)
            return;

        PushRemainingDrawCommands();

        PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, color);

        PushTextureDrawCommand(4, 6, userTextureId);
    }

    void DrawList::AddImageRounded(TextureId userTextureId, const Vec2& p1, const Vec2& p2, float rounding, uint32_t cornerFlags, const Vec2& uv1, const Vec2& uv2, Color color)
    {
        if (!color.a)
            return;

        if (rounding <= 0.0f || !cornerFlags)
            AddImage(userTextureId, p1, p2, uv1, uv2, color);
        else
        {
            PushRemainingDrawCommands();

            PathRect(p1, p2, rounding, cornerFlags);

            const size_t v0 = vertices.size(), i0 = indices.size();
            PathFillConvex(color);
            ShadeVertsLinearUV(v0, p1, p2, uv1, uv2, true);

            PushTextureDrawCommand(vertices.size() - v0, indices.size() - i0, userTextureId);
        }
    }

} // namespace nmd

namespace nmd
{
    static Context g_context;

    Context& GetContext() { return g_context; }

    void Begin()
    {
        g_context.drawList.vertices.clear();
        g_context.drawList.indices.clear();
        g_context.drawList.drawCommands.clear();
    }

    void End()
    {
        g_context.drawList.PushRemainingDrawCommands();
    }

} // namespace nmd


namespace nmd
{
	const Color Color::Black        = Color(0,   0,   0,   255);
	const Color Color::White        = Color(255, 255, 255, 255);
	const Color Color::Red          = Color(255, 0,   0,   255);
	const Color Color::Green        = Color(0,   255, 0,   255);
	const Color Color::Blue         = Color(0,   0,   255, 255);
	const Color Color::Orange       = Color(255, 165, 0,   255);
	const Color Color::Amber        = Color(255, 191, 0,   255);
	const Color Color::AndroidGreen = Color(164, 198, 57,  255);
	const Color Color::Azure        = Color(0,   127, 255, 255);
	const Color Color::Bronze       = Color(205, 127, 50,  255);
	const Color Color::Corn         = Color(251, 236, 93,  255);
	const Color Color::Emerald      = Color(80,  200, 120, 255);
	const Color Color::LapisLazuli  = Color(38,  97,  156, 255);
	const Color Color::Lava         = Color(207, 16,  32,  255);
} // namespace nmd

namespace nmd
{
#ifdef NMD_GRAPHICS_D3D9
    static LPDIRECT3DDEVICE9 g_pD3D9Device = nullptr;
    static LPDIRECT3DVERTEXBUFFER9 g_pD3D9VertexBuffer = nullptr;
    static LPDIRECT3DINDEXBUFFER9 g_pD3D9IndexBuffer = nullptr;
    static LPDIRECT3DTEXTURE9 g_pFontTexture = nullptr;
    static size_t g_D3D9VertexBufferSize, g_D3D9IndexBufferSize;
    static D3DMATRIX projectionMatrix;

    struct CustomVertex
    {
        Vec3 pos;
        D3DCOLOR color;
        Vec2 uv;

        CustomVertex() : pos(), color(), uv() {}
        CustomVertex(const Vec3& pos, D3DCOLOR color, const Vec2& uv) : pos(pos), color(color), uv(uv) {}

        static DWORD FVF;
    };

    DWORD CustomVertex::FVF = (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);

    void D3D9SetDevice(LPDIRECT3DDEVICE9 pD3D9Device) { g_pD3D9Device = pD3D9Device; }

    void D3D9Resize(int width, int height)
    {
        const float L = 0.5f;
        const float R = (float)width + 0.5f;
        const float T = 0.5f;
        const float B = (float)height + 0.5f;
        float matrix[4][4] = {
            {    2.0f / (R - L),              0.0f, 0.0f, 0.0f },
            {              0.0f,    2.0f / (T - B), 0.0f, 0.0f },
            {              0.0f,              0.0f, 0.0f, 0.0f },
            { (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f },
        };
        memcpy(&projectionMatrix, matrix, sizeof(matrix));
    }

    bool D3D9CreateFontTexture()
    {
        if (g_pFontTexture)
            g_pFontTexture->Release();

        //if (g_pD3D9Device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pFontTexture, NULL) < 0)
        //    return false;
        //
        //D3DLOCKED_RECT lockedRect;
        //if (g_pFontTexture->LockRect(0, &lockedRect, NULL, 0) != D3D_OK)
        //    return false;
        //
        //for (int y = 0; y < height; y++)
        //    memcpy((uint8_t*)lockedRect.pBits + lockedRect.Pitch * y, pixels + (width * bpp) * y, (width * bpp));
        //
        //g_pFontTexture->UnlockRect(0);

        return true;
    }

    void D3D9Render()
    {
        const Context& context = GetContext();

        // Create/recreate vertex buffer if it doesn't exist or more space is needed.
        if (!g_pD3D9VertexBuffer || g_D3D9VertexBufferSize < context.drawList.vertices.size())
        {
            if (g_pD3D9VertexBuffer)
                g_pD3D9VertexBuffer->Release(), g_pD3D9VertexBuffer = NULL;

            g_D3D9VertexBufferSize = context.drawList.vertices.size() + 5000;
            if (g_pD3D9Device->CreateVertexBuffer(static_cast<UINT>(g_D3D9VertexBufferSize * sizeof(CustomVertex)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, &g_pD3D9VertexBuffer, NULL) != D3D_OK)
                return;
        }

        // Create/recreate index buffer if it doesn't exist or more space is needed.
        if (!g_pD3D9IndexBuffer || g_D3D9IndexBufferSize < context.drawList.indices.size())
        {
            if (g_pD3D9IndexBuffer)
                g_pD3D9IndexBuffer->Release(), g_pD3D9IndexBuffer = NULL;

            g_D3D9IndexBufferSize = context.drawList.indices.size() + 10000;
            if (g_pD3D9Device->CreateIndexBuffer(static_cast<UINT>(g_D3D9IndexBufferSize * sizeof(IndexType)), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(IndexType) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pD3D9IndexBuffer, NULL) < 0)
                return;
        }
        
        void* pData = NULL;

        // Copy vertices to the gpu
        if (g_pD3D9VertexBuffer->Lock(0, static_cast<UINT>(context.drawList.vertices.size() * sizeof(CustomVertex)), &pData, D3DLOCK_DISCARD) != D3D_OK)
            return;
        size_t i = 0;
        for (auto& vertex : context.drawList.vertices)
            reinterpret_cast<CustomVertex*>(pData)[i++] = CustomVertex(Vec3(vertex.pos.x, vertex.pos.y, 0.0f), D3DCOLOR_ARGB(vertex.color.a, vertex.color.r, vertex.color.g, vertex.color.b), vertex.uv);
        g_pD3D9VertexBuffer->Unlock();

        // Copy indices to the gpu.
        if (g_pD3D9IndexBuffer->Lock(0, static_cast<UINT>(context.drawList.indices.size() * sizeof(IndexType)), &pData, D3DLOCK_DISCARD) != D3D_OK)
            return;
        memcpy(pData, context.drawList.indices.data(), context.drawList.indices.size() * sizeof(IndexType));
        g_pD3D9IndexBuffer->Unlock();
        
        // Backup current render state.
        IDirect3DStateBlock9* stateBlock;
        D3DMATRIX lastProjectionMatrix;
        if (g_pD3D9Device->CreateStateBlock(D3DSBT_ALL, &stateBlock) != D3D_OK ||
            g_pD3D9Device->GetTransform(D3DTS_PROJECTION, &lastProjectionMatrix) != D3D_OK)
            return;
        
        // Set render state.
        g_pD3D9Device->SetStreamSource(0, g_pD3D9VertexBuffer, 0, sizeof(CustomVertex));
        g_pD3D9Device->SetIndices(g_pD3D9IndexBuffer);
        g_pD3D9Device->SetFVF(CustomVertex::FVF);
        g_pD3D9Device->SetPixelShader(NULL);
        g_pD3D9Device->SetVertexShader(NULL);
        g_pD3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pD3D9Device->SetRenderState(D3DRS_LIGHTING, false);
        g_pD3D9Device->SetRenderState(D3DRS_ZENABLE, false);
        g_pD3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        g_pD3D9Device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
        g_pD3D9Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        g_pD3D9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pD3D9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        g_pD3D9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
        g_pD3D9Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        g_pD3D9Device->SetRenderState(D3DRS_FOGENABLE, false);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_pD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        g_pD3D9Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        g_pD3D9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &projectionMatrix);
        
        // Issue draw calls.
        size_t vertexBufferOffset = 0, indexBufferOffset = 0;
        for (auto& drawCommand : context.drawList.drawCommands)
        {
            g_pD3D9Device->SetTexture(0, reinterpret_cast<LPDIRECT3DTEXTURE9>(drawCommand.userTextureId));
            g_pD3D9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, static_cast<UINT>(drawCommand.numVertices), static_cast<UINT>(indexBufferOffset), static_cast<UINT>(drawCommand.numIndices / 3));
            vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
        }

        // Restore render state.
        g_pD3D9Device->SetTransform(D3DTS_PROJECTION, &lastProjectionMatrix);
        stateBlock->Apply();
        stateBlock->Release();
    }
#endif // NMD_GRAPHICS_D3D9
} // namespace nmd


namespace nmd
{

#ifdef NMD_GRAPHICS_D3D11
    static ID3D11Device* g_pD3D11Device = NULL;
    static ID3D11DeviceContext* g_pD3D11DeviceContext = NULL;
    static ID3D11Buffer* g_pD3D11VertexBuffer = NULL;
    static ID3D11Buffer* g_pD3D11IndexBuffer = NULL;
    static size_t g_D3D11VertexBufferSize = 0, g_D3D11IndexBufferSize = 0;
    static ID3DBlob* g_pD3D11ShaderBlob = NULL;
    static ID3D11VertexShader* g_pD3D11VertexShader = NULL;
    static ID3D11PixelShader* g_pD3D11PixelShader = NULL;
    static ID3D11SamplerState* g_pD3D11SamplerState = NULL;
    static ID3D11InputLayout* g_pD3D11InputLayout = NULL;
    static ID3D11Buffer* g_pD3D11VertexConstantBuffer = NULL;
    static ID3D11BlendState* g_pD3D11BlendState = NULL;
    static ID3D11RasterizerState* g_pD3D11RasterizerState = NULL;
    static ID3D11DepthStencilState* g_pD3D11DepthStencilState = NULL;
    static ID3D11ShaderResourceView* g_pD3D11FontTextureView = NULL;

    struct D3D11_RENDER_STATE
    {
        UINT ScissorRectsCount, ViewportsCount;
        D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState* RS;
        ID3D11BlendState* BlendState;
        FLOAT BlendFactor[4];
        UINT SampleMask;
        UINT StencilRef;
        ID3D11DepthStencilState* DepthStencilState;
        ID3D11ShaderResourceView* PSShaderResource;
        ID3D11SamplerState* PSSampler;
        ID3D11PixelShader* PS;
        ID3D11VertexShader* VS;
        ID3D11GeometryShader* GS;
        UINT PSInstancesCount, VSInstancesCount, GSInstancesCount;
        ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
        UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT IndexBufferFormat;
        ID3D11InputLayout* InputLayout;
    };

    struct VERTEX_CONSTANT_BUFFER { float mvp[4][4]; };

    void D3D11SetDeviceContext(ID3D11DeviceContext* pD3D11DeviceContext)
    {
        g_pD3D11DeviceContext = pD3D11DeviceContext;
        g_pD3D11DeviceContext->GetDevice(&g_pD3D11Device);
    }

    void D3D11Render()
    {
        Context& context = GetContext();

        //Don't render if screen is minimized.
        //if (context.io.displaySize.x <= 0.0f || context.io.displaySize.y <= 0.0f)
        //    return;

        //If not initialized, create pixel shader, vertex shader, input layout, etc..
        if (!g_pD3D11VertexShader)
        {
            const char* const pixelShaderCode = "\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            float4 main(PS_INPUT ps_input) : SV_TARGET\
            {\
                return ps_input.color;\
            }";

            if (D3DCompile(pixelShaderCode, strlen(pixelShaderCode), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreatePixelShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11PixelShader) != S_OK)
                return;

            const char* const vertexShaderCode = "\
            cbuffer vertexBuffer : register(b0) { float4x4 projectionMatrix; };\
            struct VS_INPUT { float2 pos : POSITION; float4 color : COLOR0; };\
            struct PS_INPUT { float4 pos : SV_POSITION; float4 color : COLOR0; };\
            PS_INPUT main(VS_INPUT vs_input)\
            {\
                PS_INPUT ps_input;\
                ps_input.pos = mul(projectionMatrix, float4(vs_input.pos.xy, 0.0f, 1.0f));\
                ps_input.color = vs_input.color;\
                return ps_input;\
            }";

            if (D3DCompile(vertexShaderCode, strlen(vertexShaderCode), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pD3D11ShaderBlob, NULL) != S_OK)
                return;

            if (g_pD3D11Device->CreateVertexShader(g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), NULL, &g_pD3D11VertexShader) != S_OK)
                return;

            D3D11_INPUT_ELEMENT_DESC inputs[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            if (g_pD3D11Device->CreateInputLayout(inputs, 2, g_pD3D11ShaderBlob->GetBufferPointer(), g_pD3D11ShaderBlob->GetBufferSize(), &g_pD3D11InputLayout) != S_OK)
                return;

            g_pD3D11ShaderBlob->Release();

            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexConstantBuffer);
        }


        //Create/recreate vertex/index buffer if it doesn't exist or more space is needed.
        {
            if (!g_pD3D11VertexBuffer || g_D3D11VertexBufferSize < context.drawList.vertices.size())
            {
                if (g_pD3D11VertexBuffer)
                    g_pD3D11VertexBuffer->Release(), g_pD3D11VertexBuffer = NULL;

                g_D3D11VertexBufferSize = context.drawList.vertices.size() + 5000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11VertexBufferSize * sizeof(Vertex));
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11VertexBuffer) != S_OK)
                    return;
            }

            if (!g_pD3D11IndexBuffer || g_D3D11IndexBufferSize < context.drawList.indices.size())
            {
                if (g_pD3D11IndexBuffer)
                    g_pD3D11IndexBuffer->Release(), g_pD3D11IndexBuffer = NULL;

                g_D3D11IndexBufferSize = context.drawList.indices.size() + 10000;

                D3D11_BUFFER_DESC desc;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = static_cast<UINT>(g_D3D11IndexBufferSize * sizeof(IndexType));
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;

                if (g_pD3D11Device->CreateBuffer(&desc, NULL, &g_pD3D11IndexBuffer) != S_OK)
                    return;
            }
        }

        //Copy data to GPU.
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource = { NULL, 0, 0 };

            //Copy vertices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            size_t i = 0;
            for (auto& vertex : context.drawList.vertices)
                memcpy(reinterpret_cast<uint8_t*>(mappedResource.pData) + i * 12, &vertex, 12), i++;
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexBuffer, 0);

            //Copy indices.
            if (g_pD3D11DeviceContext->Map(g_pD3D11IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            memcpy(mappedResource.pData, context.drawList.indices.data(), sizeof(IndexType) * context.drawList.indices.size());
            g_pD3D11DeviceContext->Unmap(g_pD3D11IndexBuffer, 0);

            //Copy matrix allowing us to specify pixel coordinates(e.g. (250, 250)) instead of normalized coordinates([-1, +1]).
            if (g_pD3D11DeviceContext->Map(g_pD3D11VertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
                return;
            const float L = 0.0f;
            const float R = 800; // 0.0f + context.io.displaySize.x;
            const float T = 0.0f;
            const float B = 600; // 0.0f + context.io.displaySize.y;
            const float mvp[4][4] =
            {
                { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
            };
            memcpy(mappedResource.pData, mvp, sizeof(mvp));
            g_pD3D11DeviceContext->Unmap(g_pD3D11VertexConstantBuffer, 0);
        }

        //Backup current render state.
        D3D11_RENDER_STATE renderState;
        {
            renderState.ScissorRectsCount = renderState.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            g_pD3D11DeviceContext->RSGetScissorRects(&renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSGetViewports(&renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSGetState(&renderState.RS);
            g_pD3D11DeviceContext->OMGetBlendState(&renderState.BlendState, renderState.BlendFactor, &renderState.SampleMask);
            g_pD3D11DeviceContext->OMGetDepthStencilState(&renderState.DepthStencilState, &renderState.StencilRef);
            g_pD3D11DeviceContext->PSGetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSGetSamplers(0, 1, &renderState.PSSampler);
            renderState.PSInstancesCount = renderState.VSInstancesCount = renderState.GSInstancesCount = 256;
            g_pD3D11DeviceContext->PSGetShader(&renderState.PS, renderState.PSInstances, &renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSGetShader(&renderState.VS, renderState.VSInstances, &renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSGetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->GSGetShader(&renderState.GS, renderState.GSInstances, &renderState.GSInstancesCount);
            g_pD3D11DeviceContext->IAGetPrimitiveTopology(&renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IAGetIndexBuffer(&renderState.IndexBuffer, &renderState.IndexBufferFormat, &renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IAGetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IAGetInputLayout(&renderState.InputLayout);
        }

        //Set our render state
        {
            D3D11_VIEWPORT vp;
            vp.Width = 800;// context.io.displaySize.x;
            vp.Height = 600;// context.io.displaySize.y;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = vp.TopLeftY = 0;
            g_pD3D11DeviceContext->RSSetViewports(1, &vp);

            const UINT stride = 12, offset = 0;
            g_pD3D11DeviceContext->IASetInputLayout(g_pD3D11InputLayout);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &g_pD3D11VertexBuffer, &stride, &offset);
            g_pD3D11DeviceContext->IASetIndexBuffer(g_pD3D11IndexBuffer, sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            g_pD3D11DeviceContext->VSSetShader(g_pD3D11VertexShader, NULL, 0);
            g_pD3D11DeviceContext->PSSetShader(g_pD3D11PixelShader, NULL, 0);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &g_pD3D11VertexConstantBuffer);
        }

        //Issue draw calls to the GPU.
        {
            size_t vertexBufferOffset = 0, indexBufferOffset = 0;
            for (auto& drawCommand : context.drawList.drawCommands)
            {
                //ID3D11ShaderResourceView* pD3D11Texture = reinterpret_cast<ID3D11ShaderResourceView*>(drawCommand.userTextureId);
                //g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &pD3D11Texture);
                g_pD3D11DeviceContext->DrawIndexed(static_cast<UINT>(drawCommand.numIndices), static_cast<UINT>(indexBufferOffset), 0/*static_cast<INT>(vertexBufferOffset)*/);
                vertexBufferOffset += drawCommand.numVertices, indexBufferOffset += drawCommand.numIndices;
            }
        }

        //Restore render state.
        {
            g_pD3D11DeviceContext->RSSetScissorRects(renderState.ScissorRectsCount, renderState.ScissorRects);
            g_pD3D11DeviceContext->RSSetViewports(renderState.ViewportsCount, renderState.Viewports);
            g_pD3D11DeviceContext->RSSetState(renderState.RS);
            g_pD3D11DeviceContext->OMSetBlendState(renderState.BlendState, renderState.BlendFactor, renderState.SampleMask);
            g_pD3D11DeviceContext->OMSetDepthStencilState(renderState.DepthStencilState, renderState.StencilRef);
            g_pD3D11DeviceContext->PSSetShaderResources(0, 1, &renderState.PSShaderResource);
            g_pD3D11DeviceContext->PSSetSamplers(0, 1, &renderState.PSSampler);
            g_pD3D11DeviceContext->PSSetShader(renderState.PS, renderState.PSInstances, renderState.PSInstancesCount);
            g_pD3D11DeviceContext->VSSetShader(renderState.VS, renderState.VSInstances, renderState.VSInstancesCount);
            g_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &renderState.VSConstantBuffer);
            g_pD3D11DeviceContext->IASetPrimitiveTopology(renderState.PrimitiveTopology);
            g_pD3D11DeviceContext->IASetIndexBuffer(renderState.IndexBuffer, renderState.IndexBufferFormat, renderState.IndexBufferOffset);
            g_pD3D11DeviceContext->IASetVertexBuffers(0, 1, &renderState.VertexBuffer, &renderState.VertexBufferStride, &renderState.VertexBufferOffset);
            g_pD3D11DeviceContext->IASetInputLayout(renderState.InputLayout);
        }
    }
#endif // NMD_GRAPHICS_D3D11

} // namespace nmd


namespace nmd
{

#ifdef NMD_GRAPHICS_OPENGL
    void OpenGLRender()
    {
        //Setup render state
        //glEnable(GL_BLEND);
        //glBlendEquation(GL_FUNC_ADD);
        ////glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glDisable(GL_CULL_FACE);
        //glDisable(GL_DEPTH_TEST);
        //glEnable(GL_SCISSOR_TEST);
        //
        //glViewport(0, 0, static_cast<GLsizei>(g_context.io.displaySize.x), static_cast<GLsizei>(g_context.io.displaySize.y));
        //float L = 0.0f;
        //float R = 0.0f + g_context.io.displaySize.x;
        //float T = 0.0f;
        //float B = 0.0f + g_context.io.displaySize.y;
        //const float ortho_projection[4][4] =
        //{
        //    { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
        //    { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
        //    { 0.0f,         0.0f,        -1.0f,   0.0f },
        //    { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
        //};
        //glUseProgram(g_ShaderHandle);
        //glUniform1i(g_AttribLocationTex, 0);
        //glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
        //
        //size_t vertexBufferOffset = 0, indexBufferOffset = 0;
        //for (auto& drawCommand : g_context.drawList.drawCommands)
        //{
        //    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numVertices) * sizeof(Vertex), reinterpret_cast<const GLvoid*>(g_context.drawList.vertices.data() + vertexBufferOffset), GL_STREAM_DRAW);
        //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCommand.numIndices) * sizeof(IndexType), reinterpret_cast<const GLvoid*>(g_context.drawList.indices.data() + indexBufferOffset), GL_STREAM_DRAW);
        //
        //    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawCommand.numIndices / 3), sizeof(IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void*>(static_cast<intptr_t>(indexBufferOffset * sizeof(IndexType))));
        //}
    }

#endif // NMD_GRAPHICS_OPENGL

} // namespace nmd

/*
Uncompressed true type font 'Karla' by Jonny Pinhorn. Licensed under the Open Font License.
Author's public contact information:
 - Jonnypinhorn.co.uk
 - https://github.com/jonpinhorn
 - https://twitter.com/jonpinhorn_type
 - jonpinhorn.typedesign@gmail.com
*/
namespace nmd
{
#ifndef NMD_GRAPHICS_DISABLE_DEFAULT_FONT
	const uint8_t karla_ttf_regular[14824] = { 0,1,0,0,0,15,0,128,0,3,0,112,71,68,69,70,0,17,0,157,0,0,53,116,0,0,0,22,71,80,79,83,3,128,47,117,0,0,53,140,0,0,4,0,71,83,85,66,220,66,234,59,0,0,57,140,0,0,0,92,79,83,47,50,132,55,30,240,0,0,47,176,0,0,0,96,99,109,97,112,78,215,82,232,0,0,48,16,0,0,0,212,103,97,115,112,0,0,0,16,0,0,53,108,0,0,0,8,103,108,121,102,180,71,175,247,0,0,0,252,0,0,42,136,104,101,97,100,249,147,22,70,0,0,44,224,0,0,0,54,104,104,101,97,6,192,3,38,0,0,47,140,0,0,0,36,104,109,116,120,54,142,32,192,0,0,45,24,0,0,2,116,108,111,99,97,16,143,27,140,0,0,43,164,0,0,1,60,109,97,120,112,0,228,0,66,0,0,43,132,0,0,0,32,110,97,109,101,68,55,103,50,0,0,48,236,0,0,3,12,112,111,115,116,67,234,186,24,0,0,51,248,0,0,1,115,112,114,101,112,104,6,140,133,0,0,48,228,0,0,0,7,0,2,0,82,255,247,0,201,2,115,0,3,0,11,0,0,19,51,3,35,22,38,52,54,50,22,20,6,99,83,9,64,6,33,33,51,35,35,2,115,254,61,185,33,47,33,33,47,33,0,0,2,0,28,1,205,1,22,2,169,0,3,0,7,0,0,19,7,35,39,51,7,35,39,110,11,60,11,250,11,60,11,2,169,220,220,220,220,0,2,0,53,0,31,2,74,2,81,0,27,0,31,0,0,37,35,7,35,55,35,55,51,55,35,55,51,55,51,7,51,55,51,7,51,7,35,7,51,7,35,7,35,39,51,55,35,1,124,154,14,74,14,99,5,100,19,101,6,100,15,74,15,154,15,73,15,103,6,102,19,103,6,103,14,74,133,153,19,153,170,139,139,54,178,53,138,138,138,138,53,178,54,139,193,178,0,0,3,0,69,255,165,2,51,2,201,0,31,0,38,0,45,0,0,1,22,22,20,6,7,21,35,53,38,38,53,51,20,23,53,39,38,38,52,54,55,53,51,21,22,22,23,35,38,39,21,23,52,39,39,21,54,54,2,6,20,22,23,23,53,1,161,69,77,114,102,62,95,121,77,139,32,83,85,101,99,62,87,94,11,78,17,97,128,92,36,63,65,247,58,40,45,30,1,74,23,77,133,101,6,81,82,8,103,87,113,13,235,12,26,72,134,89,7,75,76,7,86,68,80,12,215,186,74,29,12,220,5,62,1,178,54,73,41,14,11,199,0,0,5,0,61,255,245,2,155,2,128,0,9,0,19,0,23,0,33,0,45,0,0,18,22,21,20,6,34,38,53,52,54,22,38,34,6,21,20,22,50,54,53,37,51,1,35,1,50,22,20,6,35,34,38,52,54,23,34,6,21,20,22,51,50,54,53,52,38,241,76,75,108,73,74,118,36,58,36,36,58,36,1,25,82,254,89,81,1,170,54,75,75,54,54,73,73,54,29,36,36,29,29,36,36,2,128,76,68,68,75,76,67,67,77,97,44,44,47,47,42,42,47,133,253,139,1,20,75,137,75,76,135,76,53,44,47,47,42,42,47,47,44,0,3,0,89,255,245,3,101,2,127,0,36,0,43,0,55,0,0,1,6,7,23,22,22,50,54,55,51,6,6,35,34,38,39,39,6,35,34,38,52,54,55,39,38,53,52,54,50,22,20,6,7,23,54,55,4,6,20,22,50,55,39,55,52,38,34,6,20,22,23,23,54,55,54,2,213,45,70,43,20,31,49,39,3,74,4,67,64,39,66,39,26,100,128,105,142,95,92,14,77,105,173,103,78,88,158,68,44,254,90,73,83,172,81,186,155,55,99,55,26,30,27,104,16,6,1,94,137,85,37,17,16,37,45,72,79,32,35,22,89,105,148,75,15,13,67,80,63,84,84,119,86,38,139,82,125,102,54,96,69,66,163,200,36,48,55,64,50,26,25,48,52,17,0,0,1,0,27,1,201,0,109,2,165,0,3,0,0,19,7,35,39,109,11,60,11,2,165,220,220,0,1,0,73,255,143,1,72,2,216,0,9,0,0,18,16,23,7,38,38,52,54,55,23,155,173,57,98,100,100,98,57,2,0,254,102,175,40,89,216,231,215,90,41,0,1,0,28,255,143,1,27,2,216,0,9,0,0,54,16,39,55,22,22,20,6,7,39,201,173,57,98,100,100,98,57,102,1,154,175,41,90,215,231,216,89,40,0,0,1,0,56,1,132,1,102,2,172,0,14,0,0,19,39,55,23,39,51,7,55,23,7,23,7,39,7,39,173,117,24,106,10,62,10,106,24,116,90,44,80,80,45,2,4,40,54,54,128,128,54,54,40,89,39,104,104,39,0,0,1,0,18,0,0,1,253,1,235,0,11,0,0,1,21,51,21,35,21,35,53,35,53,51,53,1,47,206,206,79,206,206,1,235,212,67,212,212,67,212,0,0,1,0,30,255,147,0,178,0,97,0,11,0,0,22,38,52,54,50,22,20,6,7,39,54,55,82,30,31,57,38,44,45,59,62,13,3,26,44,30,45,68,71,22,29,30,45,0,0,1,0,38,1,1,1,54,1,69,0,3,0,0,19,33,21,33,38,1,16,254,240,1,69,68,0,1,0,30,255,247,0,148,0,104,0,7,0,0,22,38,52,54,50,22,20,6,63,33,33,51,34,34,9,33,47,33,33,47,33,0,0,1,0,57,255,183,1,132,2,219,0,3,0,0,1,51,3,35,1,52,80,252,79,2,219,252,220,0,0,2,0,51,255,245,2,32,2,128,0,7,0,15,0,0,0,22,16,6,34,38,16,54,4,38,34,6,20,22,50,54,1,156,132,132,228,133,133,1,22,84,161,84,84,161,84,2,128,168,254,197,168,166,1,63,166,203,131,131,245,131,131,0,0,1,0,49,0,0,0,230,2,119,0,8,0,0,19,54,55,51,17,35,17,6,7,49,67,50,64,78,50,53,2,68,9,42,253,137,2,25,32,3,0,1,0,64,0,0,2,1,2,128,0,28,0,0,55,33,21,33,53,52,55,54,55,55,54,53,52,38,34,6,7,35,54,54,50,22,20,6,7,7,6,6,21,143,1,109,254,68,84,27,33,124,96,66,119,75,6,87,5,122,191,120,79,69,127,48,47,73,73,93,111,52,17,13,50,38,87,48,61,50,50,82,88,104,153,90,27,51,20,51,41,0,0,1,0,52,255,245,2,17,2,128,0,33,0,0,1,20,7,22,22,21,20,6,34,38,53,51,22,22,50,54,52,38,35,35,53,51,50,54,52,38,34,6,7,35,54,54,50,22,1,249,110,60,74,126,206,145,82,2,91,140,77,74,83,72,49,72,84,68,112,75,13,82,11,116,183,125,1,212,100,39,17,78,61,83,101,107,97,64,68,67,99,63,63,57,96,62,39,53,83,81,97,0,0,2,0,26,0,0,2,25,2,126,0,10,0,13,0,0,1,17,51,21,35,21,35,53,33,53,1,3,51,17,1,172,109,109,77,254,187,1,73,235,233,2,126,254,94,68,152,152,61,1,169,254,87,1,44,0,0,1,0,65,255,245,1,253,2,117,0,24,0,0,37,52,35,35,19,33,21,33,7,54,51,50,22,20,6,34,38,53,51,20,23,22,51,50,54,1,171,155,169,27,1,93,254,239,17,52,47,103,118,124,196,124,79,84,26,31,64,78,204,119,1,50,72,181,9,91,184,121,99,87,82,25,7,81,0,2,0,51,255,244,2,3,2,128,0,21,0,30,0,0,1,38,34,6,21,20,23,54,54,50,22,20,6,35,34,38,16,54,51,50,22,23,4,6,20,22,50,54,53,52,38,1,169,17,180,97,5,6,101,158,114,118,96,112,138,140,121,84,106,8,254,247,75,74,114,75,75,1,214,98,148,128,53,35,92,83,111,173,107,180,1,37,179,94,76,161,71,110,67,66,58,58,66,0,0,1,0,24,0,0,1,196,2,117,0,6,0,0,1,21,1,35,1,33,53,1,196,254,224,106,1,46,254,176,2,117,73,253,212,2,41,76,0,3,0,55,255,245,2,49,2,128,0,21,0,31,0,42,0,0,19,38,53,52,54,50,22,21,20,6,7,23,22,22,21,20,6,34,38,53,52,54,23,6,6,20,22,50,54,52,38,39,39,20,22,23,23,54,54,52,38,34,6,201,125,124,205,132,69,67,17,69,74,135,229,142,81,140,63,76,96,155,91,48,42,230,44,41,73,60,78,82,130,84,1,62,57,98,75,92,98,77,52,82,10,6,26,71,61,76,92,102,77,62,78,19,7,65,96,60,52,74,45,15,216,27,39,16,28,2,59,95,59,57,0,2,0,66,255,245,2,18,2,128,0,28,0,37,0,0,1,50,22,23,22,21,20,7,6,35,34,38,53,51,22,22,50,55,54,53,52,39,6,6,34,38,53,52,54,22,34,6,21,20,22,50,54,52,1,26,51,92,33,72,136,47,55,103,120,83,2,68,104,32,91,5,10,97,157,114,118,152,114,74,75,113,75,2,128,44,41,88,141,245,69,23,112,85,57,68,19,57,200,41,33,81,80,111,86,87,106,73,66,58,58,66,71,110,0,2,0,56,0,23,0,174,1,186,0,7,0,15,0,0,54,38,52,54,50,22,20,6,2,38,52,54,50,22,20,6,89,33,33,51,34,34,51,33,33,51,34,34,23,33,47,33,33,47,33,1,50,33,47,33,33,47,33,0,0,2,0,57,255,147,0,205,1,186,0,11,0,19,0,0,22,38,52,54,50,22,20,6,7,39,54,55,2,38,52,54,50,22,20,6,109,30,31,57,38,44,45,59,62,13,17,33,33,51,34,34,3,26,44,30,45,68,71,22,29,30,45,1,78,33,47,33,33,47,33,0,0,1,0,40,0,40,1,152,2,60,0,6,0,0,1,5,5,7,37,53,37,1,152,254,224,1,32,52,254,196,1,60,2,0,207,204,61,231,69,232,0,2,0,79,0,117,1,153,1,121,0,3,0,7,0,0,37,21,33,53,37,21,33,53,1,153,254,182,1,74,254,182,185,68,68,192,69,69,0,1,0,47,0,40,1,158,2,60,0,6,0,0,19,5,21,5,39,37,37,98,1,60,254,196,51,1,30,254,226,2,60,232,69,231,61,204,207,0,0,2,0,30,255,245,1,197,2,128,0,25,0,35,0,0,19,34,7,35,54,54,50,22,21,20,6,7,6,6,7,6,21,21,35,53,52,62,2,52,38,3,50,22,20,6,35,34,38,52,54,248,117,17,84,7,108,195,113,52,58,25,34,10,19,78,42,110,38,63,87,26,34,34,26,26,32,32,2,56,92,74,90,96,69,52,69,32,14,22,14,25,47,39,45,63,67,60,45,76,51,254,46,34,47,32,32,47,34,0,0,2,0,67,255,75,3,126,2,128,0,10,0,61,0,0,1,52,35,34,6,21,20,22,50,54,55,23,20,51,50,54,55,54,53,52,38,35,34,6,7,6,16,22,51,21,34,39,38,38,52,62,2,50,30,2,21,20,6,35,34,38,39,39,6,6,35,34,38,53,52,54,51,50,22,21,2,63,59,59,94,55,82,52,23,66,39,20,47,19,46,162,139,82,134,47,100,212,185,213,134,63,68,66,118,163,182,142,100,56,129,86,43,52,5,3,18,60,40,74,95,132,91,60,69,1,33,84,97,74,53,58,47,45,45,51,27,26,61,95,119,154,56,47,100,254,214,185,64,115,54,149,178,152,111,62,54,93,126,67,122,150,36,34,16,38,48,98,75,99,135,72,73,0,0,2,0,26,0,0,2,37,2,117,0,7,0,10,0,0,19,51,19,35,39,35,7,35,55,51,3,242,96,211,82,54,250,54,83,158,209,104,2,117,253,139,160,160,222,1,54,0,0,3,0,101,0,0,2,41,2,117,0,15,0,24,0,32,0,0,19,51,50,22,21,20,6,7,22,22,21,20,7,6,35,35,55,50,54,53,52,38,35,35,21,17,51,50,54,52,38,35,35,101,235,99,107,59,60,59,71,113,42,61,236,234,72,62,68,66,155,154,58,69,67,60,154,2,117,94,71,54,80,14,10,82,52,116,41,15,69,58,48,48,57,211,1,23,57,100,56,0,0,1,0,51,255,245,2,54,2,128,0,22,0,0,1,50,22,23,7,38,38,35,34,6,20,22,51,50,54,53,51,20,6,34,38,16,54,1,63,101,123,23,85,20,85,57,79,104,93,90,75,87,85,135,235,145,150,2,128,101,81,17,59,69,132,235,142,79,65,99,116,180,1,46,169,0,0,2,0,100,0,0,2,86,2,117,0,7,0,16,0,0,1,50,22,16,6,35,35,17,19,51,50,54,53,52,38,35,35,1,29,142,171,171,142,185,79,106,105,124,125,104,106,2,117,171,254,225,171,2,117,253,208,139,107,107,137,0,0,1,0,100,0,0,1,231,2,117,0,11,0,0,19,33,21,33,21,33,21,33,21,33,21,33,100,1,131,254,204,1,34,254,222,1,52,254,125,2,117,68,211,67,214,69,0,1,0,101,0,0,1,227,2,117,0,9,0,0,19,33,21,33,21,33,21,33,17,35,101,1,126,254,209,1,26,254,230,79,2,117,68,209,68,254,228,0,0,1,0,51,255,245,2,59,2,127,0,25,0,0,37,6,34,38,16,54,51,50,22,23,7,38,34,6,20,22,51,50,54,53,53,39,53,51,17,35,1,234,42,242,155,169,126,74,111,40,76,65,174,121,104,88,72,88,158,235,64,108,119,176,1,42,176,60,66,26,81,142,227,138,96,94,2,6,52,254,201,0,0,1,0,101,0,0,2,56,2,117,0,11,0,0,19,17,33,17,51,17,35,17,33,17,35,17,180,1,53,79,79,254,203,79,2,117,254,235,1,21,253,139,1,28,254,228,2,117,0,1,0,101,0,0,0,180,2,117,0,3,0,0,19,51,17,35,101,79,79,2,117,253,139,0,0,1,0,8,255,245,1,68,2,117,0,11,0,0,36,6,34,39,53,22,50,54,53,17,51,17,1,68,93,144,79,70,123,44,79,79,90,42,82,48,60,57,1,191,254,65,0,1,0,101,0,0,2,76,2,117,0,11,0,0,19,17,1,51,1,1,35,3,7,21,35,17,181,1,25,109,254,237,1,36,102,243,62,80,2,117,254,221,1,35,254,229,254,166,1,35,64,227,2,117,0,1,0,101,0,0,1,204,2,117,0,5,0,0,55,33,21,33,17,51,180,1,24,254,153,79,69,69,2,117,0,1,0,101,0,0,2,235,2,117,0,12,0,0,27,2,51,17,35,17,3,35,3,17,35,17,214,212,210,111,79,216,53,219,79,2,117,254,86,1,170,253,139,2,5,254,74,1,189,253,244,2,117,0,0,1,0,99,0,0,2,72,2,117,0,9,0,0,19,1,17,51,17,35,1,17,35,17,193,1,56,79,88,254,195,80,2,117,254,7,1,249,253,139,2,0,254,0,2,117,0,2,0,51,255,245,2,84,2,128,0,10,0,20,0,0,1,50,22,16,6,35,34,38,53,52,54,23,34,6,20,22,51,50,54,52,38,1,67,122,151,150,123,123,149,149,123,88,102,102,88,88,103,103,2,128,167,254,193,165,167,158,159,167,71,132,248,128,128,248,132,0,0,2,0,99,0,0,2,3,2,117,0,9,0,17,0,0,0,22,20,6,35,35,21,35,17,51,17,50,54,52,38,35,35,21,1,150,109,109,104,124,79,203,65,67,67,65,124,2,117,105,178,103,243,2,117,254,194,66,115,69,250,0,0,2,0,51,255,89,2,84,2,128,0,21,0,31,0,0,5,6,35,34,39,39,38,38,16,54,51,50,22,21,20,6,7,23,22,23,50,55,1,34,6,20,22,51,50,54,52,38,2,84,40,44,85,61,47,121,147,149,123,122,151,106,91,31,29,40,43,54,254,239,88,102,102,88,88,103,103,148,19,89,67,2,166,1,60,167,167,159,132,161,23,44,39,2,25,2,119,132,248,128,128,248,132,0,0,2,0,101,0,0,2,43,2,117,0,12,0,20,0,0,0,6,7,19,35,3,35,17,35,17,51,50,22,7,50,54,52,38,35,35,21,2,27,83,81,180,102,163,110,79,219,106,113,225,71,73,71,67,140,1,115,96,14,254,251,1,1,254,255,2,117,102,215,69,113,67,249,0,0,1,0,69,255,245,2,24,2,128,0,31,0,0,1,38,35,34,6,20,22,23,23,22,22,20,6,34,38,39,51,20,22,50,54,52,38,39,39,38,38,52,54,50,22,23,1,180,17,118,63,68,42,47,131,67,79,124,208,133,2,77,87,137,80,49,48,111,78,80,112,202,107,8,1,220,92,55,77,40,15,45,23,80,145,99,105,95,62,66,61,83,48,16,41,26,72,139,93,89,75,0,0,1,0,6,0,0,1,239,2,117,0,7,0,0,19,33,21,35,17,35,17,35,6,1,233,205,79,205,2,117,68,253,207,2,49,0,0,1,0,87,255,245,2,53,2,117,0,16,0,0,19,17,20,22,51,50,54,53,17,51,17,20,6,34,38,53,17,166,88,72,72,88,79,130,219,129,2,117,254,110,88,78,78,88,1,146,254,110,118,120,120,118,1,146,0,1,0,18,0,0,2,33,2,117,0,6,0,0,27,2,51,3,35,3,101,180,181,83,223,82,222,2,117,253,239,2,17,253,139,2,117,0,0,1,0,14,0,0,3,112,2,117,0,12,0,0,27,2,51,19,19,51,3,35,3,3,35,3,102,153,155,53,154,176,87,222,83,137,142,83,199,2,117,253,245,1,202,254,54,2,11,253,139,1,147,254,109,2,117,0,0,1,0,47,0,0,2,81,2,117,0,11,0,0,33,35,39,7,35,19,3,51,19,19,51,3,2,81,98,182,169,96,222,223,95,180,166,95,217,251,251,1,56,1,61,254,255,1,1,254,196,0,0,1,0,10,0,0,2,27,2,117,0,8,0,0,27,2,51,3,17,35,17,3,104,169,173,93,226,80,223,2,117,254,223,1,33,254,141,254,254,1,2,1,115,0,0,1,0,73,0,0,2,23,2,117,0,9,0,0,55,1,37,53,33,21,1,5,21,33,73,1,107,254,149,1,206,254,149,1,107,254,50,71,1,227,3,72,71,254,29,3,72,0,0,1,0,101,255,154,1,43,2,219,0,7,0,0,19,51,21,35,17,51,21,35,101,198,119,119,198,2,219,70,253,75,70,0,0,1,0,45,255,183,1,120,2,219,0,3,0,0,19,51,19,35,45,79,252,80,2,219,252,220,0,1,255,242,255,154,0,184,2,219,0,7,0,0,23,35,53,51,17,35,53,51,184,198,119,119,198,102,70,2,181,70,0,1,0,21,0,245,1,178,2,74,0,6,0,0,55,19,51,19,35,39,7,21,176,61,176,88,119,117,245,1,85,254,171,246,246,0,0,1,0,55,255,132,2,159,255,200,0,3,0,0,23,33,21,33,55,2,104,253,152,56,68,0,0,1,0,24,2,165,1,36,3,92,0,3,0,0,19,23,7,39,57,235,23,245,3,92,133,50,113,0,0,2,0,70,255,245,1,219,1,234,0,24,0,33,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,1,141,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,58,83,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,83,0,0,2,0,101,255,245,2,22,2,169,0,13,0,23,0,0,19,54,50,22,20,6,35,34,38,39,7,35,17,51,18,6,7,21,20,22,50,54,52,38,180,48,184,122,124,89,48,77,22,18,55,79,78,76,2,79,112,82,81,1,151,83,137,225,139,48,47,84,2,169,254,251,73,65,78,66,79,99,164,98,0,1,0,50,255,245,1,205,1,234,0,21,0,0,0,22,23,7,38,38,35,34,6,20,22,50,54,53,51,20,6,35,34,38,52,54,1,89,100,15,80,13,58,35,64,79,77,115,57,81,107,86,91,127,126,1,234,87,65,7,42,47,96,165,100,53,49,77,95,138,229,134,0,2,0,60,255,245,1,236,2,169,0,13,0,23,0,0,1,17,51,17,35,39,6,6,35,34,38,52,54,50,6,6,20,22,50,54,55,53,52,38,1,158,78,64,10,23,76,45,91,123,121,185,146,79,80,112,79,2,78,1,151,1,18,253,87,80,45,46,140,225,136,70,98,164,99,75,63,78,67,78,0,2,0,50,255,245,1,215,1,234,0,16,0,24,0,0,37,50,55,51,6,6,35,34,38,52,54,50,22,7,33,20,22,19,38,35,34,6,7,51,52,1,10,99,14,81,11,105,78,96,120,121,201,99,15,254,185,72,123,23,33,61,71,6,255,53,87,72,79,137,228,136,156,112,74,95,1,106,12,83,62,100,0,1,0,49,0,0,1,91,2,177,0,20,0,0,1,38,35,34,6,21,21,51,21,35,17,35,17,35,53,51,53,52,54,50,23,1,77,33,19,35,40,99,99,78,79,79,78,102,39,2,102,13,34,43,72,54,254,88,1,168,54,73,71,67,13,0,0,3,0,28,255,10,2,37,2,59,0,40,0,52,0,62,0,0,55,38,53,52,54,55,38,53,52,54,50,23,54,54,51,7,34,7,22,21,20,6,35,34,39,6,6,21,20,51,51,50,22,21,20,6,34,38,53,52,54,23,20,22,50,54,53,52,38,35,35,6,6,0,38,34,6,21,20,22,51,50,54,121,60,47,35,54,111,152,53,7,71,58,7,88,4,35,111,83,57,44,20,32,88,186,67,78,153,234,134,51,27,83,169,113,33,34,193,51,54,1,38,62,109,62,63,54,54,62,15,29,60,31,47,8,48,78,78,96,36,53,64,78,74,43,60,77,99,23,4,29,25,53,57,51,76,114,87,66,43,54,94,38,55,73,49,23,33,2,45,1,164,65,65,52,52,67,68,0,0,1,0,101,0,0,2,7,2,169,0,17,0,0,19,54,51,50,22,21,17,35,17,52,38,34,6,21,21,35,17,51,180,48,111,85,95,78,65,122,74,79,79,1,134,100,112,97,254,231,1,25,69,70,103,91,226,2,169,0,0,2,0,94,0,0,0,198,2,178,0,3,0,11,0,0,19,51,17,35,18,38,52,54,50,22,20,6,106,79,79,15,27,27,48,29,29,1,223,254,33,2,79,27,45,27,27,45,27,0,0,2,255,170,255,10,0,213,2,178,0,9,0,23,0,0,18,38,52,54,51,50,22,20,6,35,3,50,53,17,51,17,20,7,6,35,34,39,55,22,136,27,27,24,24,29,29,24,127,88,79,90,28,34,62,72,4,63,2,79,27,45,27,27,45,27,253,0,110,2,35,253,221,137,32,10,37,69,37,0,0,1,0,101,0,0,2,20,2,169,0,11,0,0,55,7,21,35,17,51,17,55,51,7,19,35,244,64,79,79,231,110,220,231,97,253,52,201,2,169,254,116,193,179,254,213,0,1,0,101,0,0,0,180,2,169,0,3,0,0,19,51,17,35,101,79,79,2,169,253,87,0,0,1,0,100,0,0,3,82,1,234,0,30,0,0,19,54,51,50,22,23,54,51,50,22,21,17,35,17,52,38,34,6,21,21,35,17,52,38,34,6,7,21,35,17,51,177,47,114,62,84,19,45,122,86,94,78,65,122,67,78,65,120,74,2,79,68,1,130,104,61,54,115,112,97,254,231,1,25,69,70,100,88,232,1,25,69,70,98,87,235,1,222,0,1,0,100,0,0,2,6,1,234,0,17,0,0,19,54,51,50,22,21,17,35,17,52,38,34,6,7,21,35,17,51,177,47,114,85,95,78,65,120,74,2,79,68,1,130,104,112,97,254,231,1,25,69,70,98,87,235,1,222,0,2,0,50,255,245,1,240,1,234,0,11,0,19,0,0,1,50,22,21,20,6,35,34,38,53,52,54,22,38,34,6,20,22,50,54,1,17,100,123,123,100,100,123,123,243,72,139,74,70,139,76,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,0,2,0,101,255,21,2,22,1,234,0,11,0,21,0,0,19,54,50,22,20,6,34,39,17,35,17,51,18,54,52,38,34,6,7,21,20,22,175,47,187,125,126,184,44,79,65,202,85,81,116,74,2,76,1,149,85,138,226,137,69,254,219,2,201,254,93,96,167,98,73,62,91,63,72,0,0,2,0,51,255,10,2,44,1,234,0,19,0,31,0,0,1,50,23,55,51,17,20,22,22,23,7,38,38,53,53,6,34,38,52,54,22,6,20,22,51,50,55,54,55,53,52,38,1,8,102,46,17,54,23,29,21,26,68,57,41,187,126,125,37,81,86,57,82,35,12,1,76,1,234,90,78,253,209,49,33,15,7,61,18,78,74,152,87,137,226,138,70,98,167,96,73,23,32,91,66,76,0,1,0,100,0,0,1,105,1,231,0,13,0,0,1,34,7,21,35,17,51,21,54,51,50,23,7,38,1,40,113,4,79,79,36,91,29,26,4,30,1,155,178,233,1,223,92,100,9,77,10,0,0,1,0,60,255,245,1,209,1,234,0,31,0,0,18,54,50,22,23,35,38,35,34,6,20,22,23,23,22,22,20,6,34,38,39,51,22,22,50,54,52,38,39,39,38,53,76,101,172,97,2,77,9,101,49,53,40,44,96,61,65,106,190,106,3,76,2,69,104,68,40,45,93,125,1,164,70,72,55,65,44,51,35,13,32,19,59,103,83,85,67,45,45,44,61,32,12,31,37,93,0,1,0,34,255,230,1,95,2,99,0,21,0,0,37,6,39,38,39,38,53,17,35,53,51,53,51,21,51,21,35,17,20,51,50,55,1,95,86,71,51,21,11,77,77,78,150,150,68,37,46,14,40,23,16,51,25,35,1,38,60,133,133,60,254,219,73,18,0,1,0,87,255,247,1,249,1,223,0,17,0,0,37,6,35,34,38,53,17,51,17,20,51,50,54,55,53,51,17,35,1,170,49,110,81,99,79,112,65,81,2,79,79,95,104,90,94,1,48,254,214,121,100,78,241,254,33,0,0,1,0,18,0,0,1,229,1,222,0,6,0,0,27,2,51,3,35,3,107,144,145,89,190,87,190,1,222,254,117,1,139,254,34,1,222,0,0,1,0,18,0,0,2,192,1,222,0,12,0,0,27,2,51,19,19,51,3,35,3,3,35,3,100,116,114,66,113,117,78,156,77,110,108,76,159,1,222,254,131,1,102,254,150,1,129,254,34,1,79,254,177,1,222,0,0,1,0,26,0,0,1,230,1,223,0,11,0,0,33,35,39,7,35,55,39,51,23,55,51,7,1,229,96,137,130,96,181,181,96,137,131,96,183,181,181,239,240,180,180,240,0,1,0,5,255,10,1,203,1,222,0,17,0,0,23,22,51,50,55,54,55,55,3,51,19,19,51,3,6,6,34,39,5,49,54,40,29,21,17,14,196,88,145,113,80,179,23,75,115,62,139,38,40,29,59,49,1,222,254,131,1,125,253,195,73,78,35,0,0,1,0,53,0,0,1,167,1,222,0,9,0,0,55,1,33,53,33,21,1,33,21,33,53,1,25,254,231,1,114,254,235,1,21,254,142,64,1,94,64,64,254,162,64,0,0,1,0,75,255,118,1,85,2,236,0,31,0,0,19,22,21,21,20,22,51,21,34,53,53,52,38,35,35,53,51,50,54,53,53,52,55,54,51,21,34,6,21,21,20,7,154,52,61,74,213,17,19,17,17,19,17,114,40,59,74,61,52,1,47,25,88,93,81,85,69,234,92,43,36,76,35,44,91,168,49,18,69,85,82,92,86,27,0,1,0,101,255,134,0,180,2,238,0,3,0,0,19,51,17,35,101,79,79,2,238,252,152,0,0,1,0,8,255,118,1,18,2,236,0,31,0,0,19,38,53,53,52,38,35,53,50,21,21,20,22,51,51,21,35,34,6,21,21,20,7,6,35,53,50,54,53,53,52,55,195,52,61,74,213,17,19,17,17,19,17,114,41,58,74,61,52,1,51,27,86,92,82,85,69,235,91,44,35,76,36,43,92,168,48,18,69,85,81,93,88,25,0,1,0,53,0,223,1,206,1,103,0,17,0,0,19,34,7,39,54,51,50,22,51,50,55,23,6,35,34,38,39,38,158,37,24,44,33,74,34,134,26,40,25,43,35,78,23,96,14,39,1,26,59,23,111,55,57,27,106,36,6,14,0,1,0,20,0,0,2,17,2,128,0,37,0,0,1,38,35,34,6,21,21,51,21,35,21,20,7,51,50,54,55,51,6,6,35,33,53,51,50,54,53,53,35,53,51,53,52,54,51,50,22,23,1,144,16,86,42,57,182,182,37,183,49,45,6,84,6,84,94,254,187,28,38,35,91,91,93,91,71,90,11,1,234,78,64,72,109,56,104,68,26,41,44,77,77,69,44,46,108,56,109,99,109,76,74,0,0,2,0,24,3,11,1,42,3,111,0,9,0,17,0,0,18,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,53,29,29,23,23,31,31,23,146,30,30,45,31,31,3,11,29,42,29,29,42,29,29,42,29,29,42,29,0,1,0,23,2,165,1,34,3,92,0,3,0,0,1,7,39,55,1,34,244,23,235,3,22,113,50,133,0,3,0,26,0,0,2,37,3,92,0,7,0,10,0,14,0,0,19,51,19,35,39,35,7,35,55,51,3,3,23,7,39,242,96,211,82,54,250,54,83,158,209,104,99,235,23,245,2,117,253,139,160,160,222,1,54,1,72,133,50,113,0,3,0,26,0,0,2,37,3,92,0,7,0,10,0,14,0,0,19,51,19,35,39,35,7,35,55,51,3,19,7,39,55,242,96,211,82,54,250,54,83,158,209,104,134,244,23,235,2,117,253,139,160,160,222,1,54,1,2,113,50,133,0,3,0,26,0,0,2,37,3,104,0,7,0,10,0,16,0,0,19,51,19,35,39,35,7,35,55,51,3,55,7,39,7,39,55,242,96,211,82,54,250,54,83,158,209,104,180,50,129,130,50,180,2,117,253,139,160,160,222,1,54,184,39,108,108,39,156,0,0,3,0,26,0,0,2,37,3,74,0,7,0,10,0,30,0,0,19,51,19,35,39,35,7,35,55,51,3,39,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,242,96,211,82,54,250,54,83,158,209,104,68,32,25,32,12,46,52,34,46,27,26,28,10,35,11,49,41,34,60,21,2,117,253,139,160,160,222,1,54,238,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,4,0,26,0,0,2,37,3,67,0,7,0,10,0,20,0,28,0,0,19,51,19,35,39,35,7,35,55,51,3,38,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,242,96,211,82,54,250,54,83,158,209,104,86,29,29,23,23,31,31,23,146,30,30,45,31,31,2,117,253,139,160,160,222,1,54,203,29,42,29,29,42,29,29,42,29,29,42,29,0,2,0,100,0,0,1,231,3,92,0,11,0,15,0,0,19,33,21,33,21,33,21,33,21,33,21,33,19,23,7,39,100,1,131,254,204,1,34,254,222,1,52,254,125,94,235,23,245,2,117,68,211,67,214,69,3,92,133,50,113,0,0,2,0,100,0,0,1,231,3,92,0,11,0,15,0,0,51,33,53,33,53,33,53,33,53,33,53,33,37,7,39,55,100,1,131,254,204,1,34,254,222,1,52,254,125,1,71,244,23,235,69,214,67,211,68,161,113,50,133,0,0,2,0,105,0,0,1,236,3,104,0,11,0,17,0,0,19,33,21,33,21,33,21,33,21,33,21,33,1,7,39,7,39,55,105,1,131,254,204,1,34,254,222,1,52,254,125,1,118,50,129,130,50,180,2,117,68,211,67,214,69,2,204,39,108,108,39,156,0,3,0,100,0,0,1,231,3,67,0,11,0,21,0,29,0,0,19,33,21,33,21,33,21,33,21,33,21,33,18,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,100,1,131,254,204,1,34,254,222,1,52,254,125,87,29,29,23,23,31,31,23,146,30,30,45,31,31,2,117,68,211,67,214,69,2,223,29,42,29,29,42,29,29,42,29,29,42,29,0,2,0,7,0,0,1,19,3,92,0,3,0,7,0,0,19,51,17,35,3,23,7,39,101,79,79,61,235,23,245,2,117,253,139,3,92,133,50,113,0,2,0,6,0,0,1,17,3,92,0,3,0,7,0,0,51,51,17,35,55,7,39,55,101,79,79,172,244,23,235,2,117,161,113,50,133,0,0,2,255,217,0,0,1,64,3,104,0,3,0,9,0,0,19,51,17,35,19,7,39,7,39,55,101,79,79,219,50,129,130,50,180,2,117,253,139,2,204,39,108,108,39,156,0,3,0,4,0,0,1,22,3,67,0,3,0,13,0,21,0,0,19,51,17,35,2,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,101,79,79,68,29,29,23,23,31,31,23,146,30,30,45,31,31,2,117,253,139,2,223,29,42,29,29,42,29,29,42,29,29,42,29,0,0,2,0,99,0,0,2,72,3,74,0,9,0,29,0,0,19,17,51,17,1,51,17,35,17,1,55,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,99,80,1,61,88,79,254,200,81,32,25,32,12,46,52,33,47,27,26,28,10,35,11,49,41,34,60,21,2,117,253,139,2,0,254,0,2,117,254,7,1,249,141,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,0,3,0,51,255,245,2,84,3,92,0,10,0,20,0,24,0,0,1,50,22,16,6,35,34,38,53,52,54,23,34,6,20,22,51,50,54,52,38,3,23,7,39,1,67,122,151,150,123,123,149,149,123,88,102,102,88,88,103,103,188,235,23,245,2,128,167,254,193,165,167,158,159,167,71,132,248,128,128,248,132,1,35,133,50,113,0,3,0,51,255,245,2,84,3,92,0,10,0,20,0,24,0,0,1,34,6,21,20,22,51,50,54,16,38,7,50,22,20,6,35,34,38,52,54,55,7,39,55,1,67,123,149,149,123,123,150,151,122,88,103,103,88,88,102,102,221,244,23,235,2,128,167,159,158,167,165,1,63,167,71,132,248,128,128,248,132,221,113,50,133,0,0,3,0,51,255,245,2,84,3,104,0,10,0,20,0,26,0,0,1,50,22,16,6,35,34,38,53,52,54,23,34,6,20,22,51,50,54,52,38,55,7,39,7,39,55,1,67,122,151,150,123,123,149,149,123,88,102,102,88,88,103,103,91,50,129,130,50,180,2,128,167,254,193,165,167,158,159,167,71,132,248,128,128,248,132,147,39,108,108,39,156,0,0,3,0,51,255,245,2,84,3,74,0,10,0,20,0,40,0,0,1,34,6,21,20,22,51,50,54,16,38,7,50,22,20,6,35,34,38,52,54,55,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,1,67,123,149,149,123,123,150,151,122,88,103,103,88,88,102,102,19,32,25,32,12,46,52,33,47,27,26,28,10,35,11,49,41,34,60,21,2,128,167,159,158,167,165,1,63,167,71,132,248,128,128,248,132,201,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,4,0,51,255,245,2,84,3,67,0,10,0,20,0,30,0,38,0,0,1,50,22,16,6,35,34,38,53,52,54,23,34,6,20,22,51,50,54,52,46,2,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,1,67,122,151,150,123,123,149,149,123,88,102,102,88,88,103,103,196,29,29,23,23,31,31,23,146,30,30,45,31,31,2,128,167,254,193,165,167,158,159,167,71,132,248,128,128,248,132,166,29,42,29,29,42,29,29,42,29,29,42,29,0,0,2,0,87,255,245,2,53,3,92,0,16,0,20,0,0,19,17,20,22,51,50,54,53,17,51,17,20,6,34,38,53,17,55,23,7,39,166,88,72,72,88,79,130,219,129,139,235,23,245,2,117,254,110,88,78,78,88,1,146,254,110,118,120,120,118,1,146,231,133,50,113,0,2,0,87,255,245,2,53,3,92,0,16,0,20,0,0,19,17,20,22,50,54,53,17,35,17,20,6,35,34,38,53,17,37,7,39,55,87,129,219,130,79,88,72,72,88,1,37,244,23,235,2,117,254,110,118,120,120,118,1,146,254,110,88,78,78,88,1,146,161,113,50,133,0,0,2,0,87,255,245,2,53,3,104,0,16,0,22,0,0,19,17,20,22,51,50,54,53,17,51,17,20,6,34,38,53,17,37,7,39,7,39,55,166,88,72,72,88,79,130,219,129,1,162,50,129,130,50,180,2,117,254,110,88,78,78,88,1,146,254,110,118,120,120,118,1,146,87,39,108,108,39,156,0,0,3,0,87,255,245,2,53,3,67,0,16,0,26,0,34,0,0,19,17,20,22,51,50,54,53,17,51,17,20,6,34,38,53,17,54,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,166,88,72,72,88,79,130,219,129,131,29,29,23,23,31,31,23,146,30,30,45,31,31,2,117,254,110,88,78,78,88,1,146,254,110,118,120,120,118,1,146,106,29,42,29,29,42,29,29,42,29,29,42,29,0,0,3,0,60,255,245,1,209,2,219,0,24,0,33,0,37,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,3,23,7,39,1,131,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,58,83,205,235,23,245,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,83,2,86,133,50,113,0,3,0,60,255,245,1,209,2,219,0,24,0,34,0,38,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,55,19,7,39,55,1,131,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,57,82,2,28,244,23,235,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,80,63,1,212,113,50,133,0,0,3,0,60,255,245,1,209,2,231,0,24,0,33,0,39,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,19,7,39,7,39,55,1,131,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,58,83,74,50,129,130,50,180,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,83,1,198,39,108,108,39,156,0,3,0,60,255,245,1,209,2,201,0,24,0,34,0,54,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,55,3,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,1,131,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,57,82,2,174,32,25,32,12,46,52,34,46,27,26,28,10,35,11,49,41,34,60,21,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,80,63,1,192,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,4,0,60,255,245,1,209,2,193,0,24,0,33,0,43,0,51,0,0,1,52,38,34,6,21,35,52,55,54,51,50,22,21,17,35,39,6,35,34,38,52,54,50,23,21,38,34,6,21,20,51,50,54,2,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,1,131,54,103,63,87,118,36,42,89,100,68,8,44,115,77,93,103,154,70,71,106,66,102,58,83,213,29,29,23,23,31,31,23,146,30,30,45,31,31,1,54,65,56,41,44,110,26,8,90,92,254,204,80,91,86,128,72,26,50,20,38,41,90,83,1,216,29,42,29,29,42,29,29,42,29,29,42,29,0,0,3,0,51,255,245,1,216,2,224,0,16,0,24,0,28,0,0,37,50,55,51,6,6,35,34,38,52,54,50,22,7,33,20,22,19,38,35,34,6,7,51,52,3,23,7,39,1,11,99,14,81,11,105,78,96,120,121,201,99,15,254,185,72,122,22,32,62,71,6,255,227,235,23,245,53,87,72,79,137,228,136,156,112,74,95,1,106,12,83,62,100,1,98,133,50,113,0,0,3,0,51,255,245,1,216,2,224,0,16,0,22,0,26,0,0,37,6,35,34,38,53,33,54,38,34,6,20,22,51,50,54,55,2,22,21,35,54,54,55,7,39,55,1,124,14,99,65,72,1,71,15,99,201,121,120,96,78,105,11,130,55,255,6,71,184,244,23,235,140,87,95,74,112,156,136,228,137,79,72,1,31,83,62,62,83,239,113,50,133,0,3,0,54,255,245,1,219,2,236,0,16,0,24,0,30,0,0,37,50,55,51,6,6,35,34,38,52,54,50,22,7,33,20,22,19,38,35,34,6,7,51,52,55,7,39,7,39,55,1,14,99,14,81,11,105,78,96,120,121,201,99,15,254,185,72,123,23,33,61,71,6,255,53,50,129,130,50,180,53,87,72,79,137,228,136,156,112,74,95,1,106,12,83,62,100,210,39,108,108,39,156,0,4,0,51,255,245,1,216,2,193,0,16,0,24,0,34,0,42,0,0,37,50,55,51,6,6,35,34,38,52,54,50,22,7,33,20,22,19,38,35,34,6,7,51,52,38,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,1,11,99,14,81,11,105,78,96,120,121,201,99,15,254,185,72,122,22,32,62,71,6,255,234,29,29,23,23,31,31,23,146,30,30,45,31,31,53,87,72,79,137,228,136,156,112,74,95,1,106,12,83,62,100,223,29,42,29,29,42,29,29,42,29,29,42,29,0,0,2,0,12,0,0,1,24,2,219,0,3,0,7,0,0,19,51,17,35,3,23,7,39,106,79,79,61,235,23,245,1,223,254,33,2,219,133,50,113,0,2,0,11,0,0,1,22,2,219,0,3,0,7,0,0,51,51,17,35,55,7,39,55,106,79,79,172,244,23,235,1,223,182,113,50,133,0,0,2,255,222,0,0,1,69,2,231,0,3,0,9,0,0,19,51,17,35,19,7,39,7,39,55,106,79,79,219,50,129,130,50,180,1,223,254,33,2,75,39,108,108,39,156,0,3,0,9,0,0,1,27,2,193,0,3,0,13,0,21,0,0,19,51,17,35,2,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,106,79,79,68,29,29,23,23,31,31,23,146,30,30,45,31,31,1,223,254,33,2,93,29,42,29,29,42,29,29,42,29,29,42,29,0,0,2,0,101,0,0,2,7,2,201,0,17,0,37,0,0,1,34,7,39,35,17,51,53,54,54,50,22,21,17,51,17,52,38,39,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,1,83,114,47,9,68,79,2,74,120,65,78,95,187,32,25,32,12,46,52,34,46,27,26,28,10,35,11,49,41,34,60,21,1,234,104,92,254,34,235,87,98,70,69,254,231,1,25,97,112,151,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,0,3,0,51,255,245,1,241,2,219,0,11,0,19,0,23,0,0,1,50,22,21,20,6,35,34,38,53,52,54,22,38,34,6,20,22,50,54,3,23,7,39,1,18,100,123,123,100,100,123,123,243,72,139,74,70,139,76,243,235,23,245,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,2,65,133,50,113,0,0,3,0,51,255,245,1,241,2,219,0,11,0,19,0,23,0,0,1,34,6,21,20,22,51,50,54,53,52,38,6,54,50,22,20,6,34,38,1,7,39,55,1,18,100,123,123,100,100,123,123,242,74,139,72,76,139,70,1,19,244,23,235,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,1,251,113,50,133,0,3,0,51,255,245,1,241,2,231,0,11,0,19,0,25,0,0,1,50,22,21,20,6,35,34,38,53,52,54,22,38,34,6,20,22,50,54,19,7,39,7,39,55,1,18,100,123,123,100,100,123,123,243,72,139,74,70,139,76,36,50,129,130,50,180,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,1,177,39,108,108,39,156,0,0,3,0,51,255,245,1,241,2,201,0,11,0,19,0,39,0,0,1,34,6,21,20,22,51,50,54,53,52,38,6,54,50,22,20,6,34,38,19,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,1,18,100,123,123,100,100,123,123,242,74,139,72,76,139,70,74,32,25,32,12,46,52,33,47,27,26,28,10,35,11,49,41,34,60,21,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,1,231,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,4,0,51,255,245,1,241,2,193,0,11,0,19,0,29,0,37,0,0,1,50,22,21,20,6,35,34,38,53,52,54,22,38,34,6,20,22,50,54,2,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,1,18,100,123,123,100,100,123,123,243,72,139,74,70,139,76,250,29,29,23,23,31,31,23,146,30,30,45,31,31,1,234,130,121,120,130,130,120,121,130,165,97,97,171,97,97,1,195,29,42,29,29,42,29,29,42,29,29,42,29,0,2,0,87,255,247,1,249,3,1,0,17,0,21,0,0,37,6,35,34,38,53,17,51,17,20,51,50,54,55,53,51,17,35,3,23,7,39,1,170,49,110,81,99,79,112,65,81,2,79,79,226,235,23,245,95,104,90,94,1,48,254,214,121,100,78,241,254,33,3,1,133,50,113,0,2,0,87,255,247,1,249,3,1,0,17,0,21,0,0,5,50,55,21,51,17,35,21,20,6,35,34,53,17,35,17,20,22,19,7,39,55,1,11,110,49,79,79,82,66,112,79,99,247,244,23,235,9,104,95,1,223,233,83,103,121,1,42,254,208,94,90,2,196,113,50,133,0,0,2,0,87,255,247,1,249,3,13,0,17,0,23,0,0,37,6,35,34,38,53,17,51,17,20,51,50,54,55,53,51,17,35,19,7,39,7,39,55,1,170,49,110,81,99,79,112,65,81,2,79,79,54,50,129,130,50,180,95,104,90,94,1,48,254,214,121,100,78,241,254,33,2,113,39,108,108,39,156,0,3,0,87,255,247,1,249,2,193,0,17,0,27,0,35,0,0,37,6,35,34,38,53,17,51,17,20,51,50,54,55,53,51,17,35,2,38,52,54,51,50,22,20,6,35,50,38,52,54,50,22,20,6,1,170,49,110,81,99,79,112,65,81,2,79,79,233,29,29,23,23,31,31,23,146,30,30,45,31,31,95,104,90,94,1,48,254,214,121,100,78,241,254,33,2,93,29,42,29,29,42,29,29,42,29,29,42,29,0,0,1,0,101,0,0,0,180,1,223,0,3,0,0,19,51,17,35,101,79,79,1,223,254,33,0,0,1,0,36,1,163,0,184,2,113,0,11,0,0,18,38,52,54,50,22,20,6,7,39,54,55,88,30,31,57,38,44,45,59,62,13,2,13,26,44,30,45,68,71,22,29,30,45,0,1,0,24,2,165,1,127,3,104,0,5,0,0,1,7,39,7,39,55,1,127,50,129,130,50,180,2,204,39,108,108,39,156,0,1,0,23,2,207,1,83,3,74,0,19,0,0,19,34,7,39,54,54,50,23,22,22,50,54,55,23,6,6,34,46,2,112,32,25,32,12,46,52,33,47,27,26,28,10,35,11,49,41,34,60,21,3,2,51,19,51,53,17,23,12,25,27,23,48,52,12,31,8,0,1,0,55,1,1,1,163,1,69,0,3,0,0,19,33,21,33,55,1,108,254,148,1,69,68,0,1,0,55,1,1,2,159,1,69,0,3,0,0,19,33,21,33,55,2,104,253,152,1,69,68,0,1,0,36,1,162,0,184,2,114,0,12,0,0,18,6,34,38,52,54,55,23,6,7,22,22,21,162,31,56,39,47,45,56,62,13,23,30,1,193,31,46,69,71,22,29,32,44,2,26,22,0,0,1,0,15,1,162,0,162,2,114,0,13,0,0,18,54,50,22,21,20,6,7,39,54,55,38,38,53,36,32,57,37,47,44,56,62,13,23,31,2,84,30,44,35,35,71,23,30,32,44,2,26,22,0,0,1,0,92,0,0,1,252,2,117,0,27,0,0,19,33,21,35,22,23,51,21,35,6,7,6,7,5,35,37,53,51,50,54,55,35,53,51,38,38,35,35,92,1,160,183,69,15,99,94,7,112,40,55,1,18,112,254,242,61,87,84,3,235,231,11,83,76,61,2,117,49,36,67,48,115,44,15,5,250,248,48,76,57,48,46,57,0,0,1,0,79,1,2,1,153,1,69,0,3,0,0,1,21,33,53,1,153,254,182,1,69,67,67,0,1,0,0,0,157,0,63,0,5,0,0,0,0,0,2,0,0,0,1,0,1,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,0,44,0,93,0,164,0,235,1,66,1,79,1,101,1,123,1,153,1,174,1,198,1,211,1,229,1,243,2,19,2,39,2,85,2,136,2,164,2,203,2,253,3,16,3,82,3,139,3,169,3,205,3,225,3,244,4,8,4,63,4,150,4,174,4,224,5,6,5,37,5,60,5,81,5,122,5,146,5,159,5,182,5,209,5,224,5,251,6,18,6,54,6,85,6,137,6,174,6,225,6,243,7,16,7,35,7,64,7,90,7,112,7,136,7,153,7,166,7,182,7,200,7,213,7,227,8,22,8,62,8,98,8,138,8,179,8,212,9,45,9,75,9,100,9,140,9,163,9,176,9,222,9,252,10,30,10,68,10,119,10,145,10,195,10,229,11,3,11,22,11,51,11,74,11,108,11,131,11,175,11,188,11,232,12,7,12,60,12,91,12,105,12,136,12,167,12,201,12,253,13,45,13,76,13,106,13,140,13,188,13,208,13,227,13,250,14,32,14,84,14,127,14,170,14,216,15,24,15,84,15,120,15,157,15,197,15,251,16,53,16,113,16,174,16,255,17,75,17,124,17,170,17,221,18,31,18,51,18,70,18,93,18,131,18,191,18,233,19,19,19,64,19,127,19,186,19,223,20,4,20,44,20,99,20,112,20,136,20,153,20,188,20,201,20,214,20,240,21,11,21,55,21,68,0,1,0,0,0,1,0,0,184,134,128,40,95,15,60,245,0,11,3,232,0,0,0,0,202,188,123,31,0,0,0,0,204,143,87,201,255,170,255,10,3,126,3,111,0,0,0,8,0,2,0,0,0,0,0,0,2,41,0,0,0,0,0,0,2,41,0,0,0,238,0,0,1,22,0,82,1,50,0,28,2,115,0,53,2,120,0,69,2,214,0,61,3,148,0,89,0,136,0,27,1,100,0,73,1,100,0,28,1,156,0,56,2,15,0,18,0,210,0,30,1,93,0,38,0,179,0,30,1,176,0,57,2,82,0,51,1,75,0,49,2,68,0,64,2,79,0,52,2,47,0,26,2,53,0,65,2,61,0,51,1,214,0,24,2,103,0,55,2,69,0,66,0,231,0,56,1,6,0,57,1,198,0,40,1,232,0,79,1,198,0,47,1,243,0,30,3,172,0,67,2,63,0,26,2,110,0,101,2,101,0,51,2,149,0,100,2,50,0,100,2,12,0,101,2,133,0,51,2,157,0,101,1,24,0,101,1,148,0,8,2,97,0,101,1,212,0,101,3,80,0,101,2,173,0,99,2,134,0,51,2,42,0,99,2,140,0,51,2,101,0,101,2,93,0,69,1,245,0,6,2,142,0,87,2,51,0,18,3,127,0,14,2,130,0,47,2,37,0,10,2,80,0,73,1,61,0,101,1,176,0,45,1,29,255,242,1,192,0,21,2,214,0,55,1,59,0,24,2,48,0,70,2,72,0,101,2,3,0,50,2,82,0,60,2,3,0,50,1,87,0,49,2,53,0,28,2,92,0,101,1,43,0,94,1,58,255,170,2,52,0,101,1,24,0,101,3,167,0,100,2,91,0,100,2,34,0,50,2,82,0,101,2,73,0,51,1,125,0,100,2,14,0,60,1,120,0,34,2,94,0,87,1,247,0,18,2,213,0,18,2,0,0,26,1,228,0,5,1,219,0,53,1,93,0,75,1,24,0,101,1,93,0,8,2,3,0,53,2,53,0,20,1,67,0,24,1,59,0,23,2,63,0,26,2,63,0,26,2,63,0,26,2,63,0,26,2,63,0,26,2,52,0,100,2,52,0,100,2,52,0,105,2,52,0,100,1,24,0,7,1,24,0,6,1,24,255,217,1,24,0,4,2,174,0,99,2,134,0,51,2,134,0,51,2,134,0,51,2,134,0,51,2,134,0,51,2,142,0,87,2,142,0,87,2,142,0,87,2,142,0,87,2,39,0,60,2,39,0,60,2,39,0,60,2,39,0,60,2,39,0,60,2,2,0,51,2,2,0,51,2,2,0,54,2,2,0,51,1,43,0,12,1,43,0,11,1,43,255,222,1,43,0,9,2,92,0,101,2,35,0,51,2,35,0,51,2,35,0,51,2,35,0,51,2,35,0,51,2,94,0,87,2,94,0,87,2,94,0,87,2,94,0,87,1,24,0,101,0,184,0,36,1,151,0,24,1,110,0,23,1,218,0,55,2,214,0,55,0,214,0,36,0,214,0,15,2,69,0,92,1,232,0,79,0,1,0,0,3,149,255,4,0,0,3,172,255,170,255,216,3,126,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,0,3,1,253,1,144,0,5,0,8,2,188,2,138,0,0,0,140,2,188,2,138,0,0,1,221,0,50,0,250,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,39,0,0,0,66,0,0,0,0,0,0,0,0,112,121,114,115,0,64,0,32,34,18,3,149,255,4,0,0,3,149,0,252,0,0,0,1,0,0,0,0,1,222,2,117,0,0,0,32,0,2,0,0,0,2,0,0,0,3,0,0,0,20,0,3,0,1,0,0,0,20,0,4,0,192,0,0,0,44,0,32,0,4,0,12,0,126,0,160,0,163,0,168,0,180,0,196,0,207,0,214,0,220,0,228,0,239,0,246,0,252,1,49,2,188,2,198,2,220,32,20,32,25,32,185,34,18,255,255,0,0,0,32,0,160,0,163,0,168,0,180,0,192,0,200,0,209,0,217,0,224,0,232,0,241,0,249,1,49,2,188,2,198,2,220,32,19,32,24,32,185,34,18,255,255,255,227,255,99,255,191,255,187,255,176,255,165,255,162,255,161,255,159,255,156,255,153,255,152,255,150,255,98,253,216,253,207,253,186,224,132,224,129,223,226,222,138,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,184,1,255,133,176,4,141,0,0,0,0,11,0,138,0,3,0,1,4,9,0,0,0,218,0,0,0,3,0,1,4,9,0,1,0,10,0,218,0,3,0,1,4,9,0,2,0,14,0,228,0,3,0,1,4,9,0,3,0,84,0,242,0,3,0,1,4,9,0,4,0,10,0,218,0,3,0,1,4,9,0,5,0,26,1,70,0,3,0,1,4,9,0,6,0,26,1,96,0,3,0,1,4,9,0,9,0,32,1,122,0,3,0,1,4,9,0,12,0,28,1,154,0,3,0,1,4,9,0,13,0,152,1,182,0,3,0,1,4,9,0,14,0,52,2,78,0,67,0,111,0,112,0,121,0,114,0,105,0,103,0,104,0,116,0,32,0,40,0,99,0,41,0,32,0,50,0,48,0,49,0,49,0,45,0,50,0,48,0,49,0,50,0,44,0,32,0,74,0,111,0,110,0,97,0,116,0,104,0,97,0,110,0,32,0,80,0,105,0,110,0,104,0,111,0,114,0,110,0,32,0,40,0,106,0,111,0,110,0,112,0,105,0,110,0,104,0,111,0,114,0,110,0,46,0,116,0,121,0,112,0,101,0,100,0,101,0,115,0,105,0,103,0,110,0,64,0,103,0,109,0,97,0,105,0,108,0,46,0,99,0,111,0,109,0,41,0,44,0,32,0,119,0,105,0,116,0,104,0,32,0,82,0,101,0,115,0,101,0,114,0,118,0,101,0,100,0,32,0,70,0,111,0,110,0,116,0,32,0,78,0,97,0,109,0,101,0,115,0,32,0,39,0,75,0,97,0,114,0,108,0,97,0,39,0,75,0,97,0,114,0,108,0,97,0,82,0,101,0,103,0,117,0,108,0,97,0,114,0,70,0,111,0,110,0,116,0,70,0,111,0,114,0,103,0,101,0,32,0,50,0,46,0,48,0,32,0,58,0,32,0,75,0,97,0,114,0,108,0,97,0,32,0,82,0,101,0,103,0,117,0,108,0,97,0,114,0,32,0,58,0,32,0,49,0,51,0,45,0,49,0,48,0,45,0,50,0,48,0,49,0,49,0,86,0,101,0,114,0,115,0,105,0,111,0,110,0,32,0,49,0,46,0,48,0,48,0,48,0,75,0,97,0,114,0,108,0,97,0,45,0,82,0,101,0,103,0,117,0,108,0,97,0,114,0,74,0,111,0,110,0,97,0,116,0,104,0,97,0,110,0,32,0,80,0,105,0,110,0,104,0,111,0,114,0,110,0,106,0,111,0,110,0,112,0,105,0,110,0,104,0,111,0,114,0,110,0,46,0,99,0,111,0,109,0,84,0,104,0,105,0,115,0,32,0,70,0,111,0,110,0,116,0,32,0,83,0,111,0,102,0,116,0,119,0,97,0,114,0,101,0,32,0,105,0,115,0,32,0,108,0,105,0,99,0,101,0,110,0,115,0,101,0,100,0,32,0,117,0,110,0,100,0,101,0,114,0,32,0,116,0,104,0,101,0,32,0,83,0,73,0,76,0,32,0,79,0,112,0,101,0,110,0,32,0,70,0,111,0,110,0,116,0,32,0,76,0,105,0,99,0,101,0,110,0,115,0,101,0,44,0,32,0,86,0,101,0,114,0,115,0,105,0,111,0,110,0,32,0,49,0,46,0,49,0,46,0,104,0,116,0,116,0,112,0,58,0,47,0,47,0,115,0,99,0,114,0,105,0,112,0,116,0,115,0,46,0,115,0,105,0,108,0,46,0,111,0,114,0,103,0,47,0,79,0,70,0,76,0,2,0,0,0,0,0,0,255,181,0,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,0,0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0,11,0,12,0,13,0,14,0,15,0,16,0,17,0,18,0,19,0,20,0,21,0,22,0,23,0,24,0,25,0,26,0,27,0,28,0,29,0,30,0,31,0,32,0,33,0,34,0,35,0,36,0,37,0,38,0,39,0,40,0,41,0,42,0,43,0,44,0,45,0,46,0,47,0,48,0,49,0,50,0,51,0,52,0,53,0,54,0,55,0,56,0,57,0,58,0,59,0,60,0,61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,0,74,0,75,0,76,0,77,0,78,0,79,0,80,0,81,0,82,0,83,0,84,0,85,0,86,0,87,0,88,0,89,0,90,0,91,0,92,0,93,0,94,0,95,0,96,0,97,0,133,0,142,0,141,0,173,0,201,0,199,0,174,0,98,0,203,0,101,0,200,0,202,0,207,0,204,0,205,0,206,0,102,0,211,0,208,0,209,0,175,0,103,0,214,0,212,0,213,0,104,0,106,0,105,0,107,0,109,0,108,0,113,0,112,0,114,0,115,0,117,0,116,0,118,0,119,0,120,0,122,0,121,0,123,0,125,0,124,0,127,0,126,0,128,0,129,0,215,1,2,0,216,0,217,0,178,0,179,0,182,0,183,1,3,0,239,10,97,112,111,115,116,114,111,112,104,101,11,114,117,112,101,101,115,121,109,98,111,108,0,0,1,0,1,255,255,0,15,0,1,0,0,0,12,0,0,0,0,0,0,0,2,0,1,0,1,0,156,0,1,0,0,0,1,0,0,0,10,0,30,0,44,0,1,108,97,116,110,0,8,0,4,0,0,0,0,255,255,0,1,0,0,0,1,107,101,114,110,0,8,0,0,0,1,0,0,0,1,0,4,0,2,0,0,0,1,0,8,0,1,0,82,0,4,0,0,0,36,0,152,0,186,0,192,1,6,1,40,1,54,1,60,1,66,1,72,1,158,1,216,2,14,2,20,2,130,2,148,2,162,2,176,2,190,2,216,2,226,2,236,2,242,3,0,3,10,3,20,3,38,3,48,3,62,3,76,3,102,3,108,3,118,3,140,3,158,3,168,3,190,0,2,0,11,0,36,0,36,0,0,0,39,0,39,0,1,0,41,0,41,0,2,0,47,0,47,0,3,0,50,0,52,0,4,0,54,0,55,0,7,0,57,0,60,0,9,0,68,0,70,0,13,0,72,0,75,0,16,0,77,0,78,0,20,0,80,0,93,0,22,0,8,0,55,255,216,0,57,255,228,0,58,255,228,0,60,255,230,0,73,255,244,0,89,255,234,0,90,255,232,0,92,255,228,0,1,0,60,255,231,0,17,0,36,255,199,0,58,255,248,0,59,255,240,0,60,255,248,0,68,255,232,0,70,255,228,0,71,255,232,0,72,255,228,0,74,255,211,0,82,255,228,0,84,255,228,0,88,255,232,0,89,255,224,0,90,255,220,0,91,255,228,0,92,255,228,0,93,255,232,0,8,0,45,0,16,0,55,255,207,0,57,255,211,0,58,255,215,0,60,255,187,0,89,255,232,0,90,255,232,0,92,255,236,0,3,0,57,255,248,0,59,255,244,0,60,255,240,0,1,0,36,255,218,0,1,0,60,255,240,0,1,0,60,255,230,0,21,0,36,255,216,0,45,255,187,0,68,255,169,0,70,255,181,0,71,255,171,0,72,255,181,0,73,255,203,0,74,255,169,0,80,255,187,0,81,255,187,0,82,255,177,0,83,255,187,0,84,255,181,0,85,255,183,0,86,255,159,0,88,255,187,0,89,255,183,0,90,255,216,0,91,255,199,0,92,255,206,0,93,255,203,0,14,0,36,255,223,0,38,255,248,0,45,255,187,0,50,255,248,0,68,255,223,0,70,255,223,0,71,255,223,0,72,255,223,0,73,255,216,0,74,255,207,0,82,255,223,0,84,255,227,0,85,255,239,0,86,255,223,0,13,0,36,255,223,0,45,255,187,0,68,255,223,0,70,255,223,0,71,255,223,0,72,255,223,0,73,255,216,0,74,255,207,0,82,255,223,0,84,255,223,0,85,255,223,0,86,255,216,0,88,255,228,0,1,0,50,255,244,0,27,0,36,255,187,0,38,255,239,0,43,255,248,0,45,255,174,0,50,255,240,0,52,255,240,0,54,255,228,0,68,255,195,0,70,255,191,0,71,255,191,0,72,255,191,0,73,255,191,0,74,255,179,0,80,255,191,0,81,255,191,0,82,255,191,0,83,255,191,0,84,255,191,0,85,255,191,0,86,255,191,0,87,255,219,0,88,255,189,0,89,255,211,0,90,255,211,0,91,255,203,0,92,255,199,0,93,255,191,0,4,0,55,255,167,0,57,255,219,0,58,255,228,0,60,255,203,0,3,0,55,255,236,0,57,255,227,0,60,255,239,0,3,0,55,255,195,0,57,255,228,0,60,255,223,0,3,0,55,255,195,0,57,255,232,0,60,255,215,0,6,0,36,255,240,0,45,255,203,0,70,255,240,0,74,255,230,0,77,255,240,0,86,255,232,0,2,0,77,0,73,0,92,0,16,0,2,0,55,255,248,0,60,255,211,0,1,0,73,255,234,0,3,0,73,255,240,0,77,255,248,0,92,255,240,0,2,0,55,255,220,0,60,255,215,0,2,0,55,255,212,0,60,255,215,0,4,0,55,255,185,0,57,255,223,0,58,255,223,0,60,255,191,0,2,0,55,255,203,0,60,255,220,0,3,0,55,255,183,0,60,255,209,0,77,0,81,0,3,0,45,255,216,0,55,255,208,0,74,255,236,0,6,0,55,255,167,0,57,255,228,0,58,255,220,0,60,255,199,0,73,255,228,0,92,255,236,0,1,0,60,255,219,0,2,0,55,255,199,0,60,255,207,0,5,0,36,255,232,0,45,255,203,0,55,255,196,0,60,255,211,0,74,255,248,0,4,0,36,255,228,0,45,255,215,0,55,255,224,0,60,255,224,0,2,0,55,255,215,0,60,255,207,0,5,0,36,255,244,0,45,255,232,0,55,255,200,0,60,255,228,0,74,255,248,0,2,0,55,255,232,0,60,255,215,0,1,0,0,0,10,0,30,0,44,0,1,108,97,116,110,0,8,0,4,0,0,0,0,255,255,0,1,0,0,0,1,111,110,117,109,0,8,0,0,0,1,0,0,0,1,0,4,0,1,0,0,0,1,0,8,0,2,0,26,0,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,0,19,0,28,0,0 };
#endif // NMD_GRAPHICS_DISABLE_DEFAULT_FONT
} // namespace nmd

// stb_truetype.h - v1.24 - public domain
// authored from 2009-2020 by Sean Barrett / RAD Game Tools
//
// =======================================================================
//
//    NO SECURITY GUARANTEE -- DO NOT USE THIS ON UNTRUSTED FONT FILES
//
// This library does no range checking of the offsets found in the file,
// meaning an attacker can use it to read arbitrary memory.
//
// =======================================================================
//
//   This library processes TrueType files:
//        parse files
//        extract glyph metrics
//        extract glyph shapes
//        render glyphs to one-channel bitmaps with antialiasing (box filter)
//        render glyphs to one-channel SDF bitmaps (signed-distance field/function)
//
//   Todo:
//        non-MS cmaps
//        crashproof on bad data
//        hinting? (no longer patented)
//        cleartype-style AA?
//        optimize: use simple memory allocator for intermediates
//        optimize: build edge-list directly from curves
//        optimize: rasterize directly from curves?
//
// ADDITIONAL CONTRIBUTORS
//
//   Mikko Mononen: compound shape support, more cmap formats
//   Tor Andersson: kerning, subpixel rendering
//   Dougall Johnson: OpenType / Type 2 font handling
//   Daniel Ribeiro Maciel: basic GPOS-based kerning
//
//   Misc other:
//       Ryan Gordon
//       Simon Glass
//       github:IntellectualKitty
//       Imanol Celaya
//       Daniel Ribeiro Maciel
//
//   Bug/warning reports/fixes:
//       "Zer" on mollyrocket       Fabian "ryg" Giesen   github:NiLuJe
//       Cass Everitt               Martins Mozeiko       github:aloucks
//       stoiko (Haemimont Games)   Cap Petschulat        github:oyvindjam
//       Brian Hook                 Omar Cornut           github:vassvik
//       Walter van Niftrik         Ryan Griege
//       David Gow                  Peter LaValle
//       David Given                Sergey Popov
//       Ivan-Assen Ivanov          Giumo X. Clanjor
//       Anthony Pesch              Higor Euripedes
//       Johan Duparc               Thomas Fields
//       Hou Qiming                 Derek Vinyard
//       Rob Loach                  Cort Stratton
//       Kenney Phillis Jr.         Brian Costabile            
//       Ken Voskuil (kaesve)       
//
// VERSION HISTORY
//
//   1.24 (2020-02-05) fix warning
//   1.23 (2020-02-02) query SVG data for glyphs; query whole kerning table (but only kern not GPOS)
//   1.22 (2019-08-11) minimize missing-glyph duplication; fix kerning if both 'GPOS' and 'kern' are defined
//   1.21 (2019-02-25) fix warning
//   1.20 (2019-02-07) PackFontRange skips missing codepoints; GetScaleFontVMetrics()
//   1.19 (2018-02-11) GPOS kerning, STBTT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) user-defined fabs(); rare memory leak; remove duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use allocation userdata properly
//   1.08 (2015-09-13) document stbtt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     variant PackFontRanges to pack and render in separate phases;
//                     fix stbtt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with STBTT_assert() in new rasterizer
//
//   Full history can be found at the end of this file.
//
// LICENSE
//
//   See end of file for license information.
//
// USAGE
//
//   Include this file in whatever places need to refer to it. In ONE C/C++
//   file, write:
//      #define STB_TRUETYPE_IMPLEMENTATION
//   before the #include of this file. This expands out the actual
//   implementation into that C/C++ file.
//
//   To make the implementation private to the file that generates the implementation,
//      #define STBTT_STATIC
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           stbtt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           stbtt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           #include "stb_rect_pack.h"           -- optional, but you really want it
//           stbtt_PackBegin()
//           stbtt_PackSetOversampling()          -- for improved quality on small fonts
//           stbtt_PackFontRanges()               -- pack and renders
//           stbtt_PackEnd()
//           stbtt_GetPackedQuad()
//
//   "Load" a font file from a memory buffer (you have to keep the buffer loaded)
//           stbtt_InitFont()
//           stbtt_GetFontOffsetForIndex()        -- indexing for TTC font collections
//           stbtt_GetNumberOfFonts()             -- number of fonts for TTC font collections
//
//   Render a unicode codepoint to a bitmap
//           stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
//           stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide
//           stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
//
//   Character advance/positioning
//           stbtt_GetCodepointHMetrics()
//           stbtt_GetFontVMetrics()
//           stbtt_GetFontVMetricsOS2()
//           stbtt_GetCodepointKernAdvance()
//
//   Starting with version 1.06, the rasterizer was replaced with a new,
//   faster and generally-more-precise rasterizer. The new rasterizer more
//   accurately measures pixel coverage for anti-aliasing, except in the case
//   where multiple shapes overlap, in which case it overestimates the AA pixel
//   coverage. Thus, anti-aliasing of intersecting shapes may look wrong. If
//   this turns out to be a problem, you can re-enable the old rasterizer with
//        #define STBTT_RASTERIZER_VERSION 1
//   which will incur about a 15% speed hit.
//
// ADDITIONAL DOCUMENTATION
//
//   Immediately after this block comment are a series of sample programs.
//
//   After the sample programs is the "header file" section. This section
//   includes documentation for each API function.
//
//   Some important concepts to understand to use this library:
//
//      Codepoint
//         Characters are defined by unicode codepoints, e.g. 65 is
//         uppercase A, 231 is lowercase c with a cedilla, 0x7e30 is
//         the hiragana for "ma".
//
//      Glyph
//         A visual character shape (every codepoint is rendered as
//         some glyph)
//
//      Glyph index
//         A font-specific integer ID representing a glyph
//
//      Baseline
//         Glyph shapes are defined relative to a baseline, which is the
//         bottom of uppercase characters. Characters extend both above
//         and below the baseline.
//
//      Current Point
//         As you draw text to the screen, you keep track of a "current point"
//         which is the origin of each character. The current point's vertical
//         position is the baseline. Even "baked fonts" use this model.
//
//      Vertical Font Metrics
//         The vertical qualities of the font, used to vertically position
//         and space the characters. See docs for stbtt_GetFontVMetrics.
//
//      Font Size in Pixels or Points
//         The preferred interface for specifying font sizes in stb_truetype
//         is to specify how tall the font's vertical extent should be in pixels.
//         If that sounds good enough, skip the next paragraph.
//
//         Most font APIs instead use "points", which are a common typographic
//         measurement for describing font size, defined as 72 points per inch.
//         stb_truetype provides a point API for compatibility. However, true
//         "per inch" conventions don't make much sense on computer displays
//         since different monitors have different number of pixels per
//         inch. For example, Windows traditionally uses a convention that
//         there are 96 pixels per inch, thus making 'inch' measurements have
//         nothing to do with inches, and thus effectively defining a point to
//         be 1.333 pixels. Additionally, the TrueType font data provides
//         an explicit scale factor to scale a given font's glyphs to points,
//         but the author has observed that this scale factor is often wrong
//         for non-commercial fonts, thus making fonts scaled in points
//         according to the TrueType spec incoherently sized in practice.
//
// DETAILED USAGE:
//
//  Scale:
//    Select how high you want the font to be, in points or pixels.
//    Call ScaleForPixelHeight or ScaleForMappingEmToPixels to compute
//    a scale factor SF that will be used by all other functions.
//
//  Baseline:
//    You need to select a y-coordinate that is the baseline of where
//    your text will appear. Call GetFontBoundingBox to get the baseline-relative
//    bounding box for all characters. SF*-y0 will be the distance in pixels
//    that the worst-case character could extend above the baseline, so if
//    you want the top edge of characters to appear at the top of the
//    screen where y=0, then you would set the baseline to SF*-y0.
//
//  Current point:
//    Set the current point where the first character will appear. The
//    first character could extend left of the current point; this is font
//    dependent. You can either choose a current point that is the leftmost
//    point and hope, or add some padding, or check the bounding box or
//    left-side-bearing of the first character to be displayed and set
//    the current point based on that.
//
//  Displaying a character:
//    Compute the bounding box of the character. It will contain signed values
//    relative to <current_point, baseline>. I.e. if it returns x0,y0,x1,y1,
//    then the character should be displayed in the rectangle from
//    <current_point+SF*x0, baseline+SF*y0> to <current_point+SF*x1,baseline+SF*y1).
//
//  Advancing for the next character:
//    Call GlyphHMetrics, and compute 'current_point += SF * advance'.
//
//
// ADVANCED USAGE
//
//   Quality:
//
//    - Use the functions with Subpixel at the end to allow your characters
//      to have subpixel positioning. Since the font is anti-aliased, not
//      hinted, this is very import for quality. (This is not possible with
//      baked fonts.)
//
//    - Kerning is now supported, and if you're supporting subpixel rendering
//      then kerning is worth using to give your text a polished look.
//
//   Performance:
//
//    - Convert Unicode codepoints to glyph indexes and operate on the glyphs;
//      if you don't do this, stb_truetype is forced to do the conversion on
//      every call.
//
//    - There are a lot of memory allocations. We should modify it to take
//      a temp buffer and allocate from the temp buffer (without freeing),
//      should help performance a lot.
//
// NOTES
//
//   The system uses the raw data found in the .ttf file without changing it
//   and without building auxiliary data structures. This is a bit inefficient
//   on little-endian systems (the data is big-endian), but assuming you're
//   caching the bitmaps or glyph shapes this shouldn't be a big deal.
//
//   It appears to be very hard to programmatically determine what font a
//   given file is in a general way. I provide an API for this, but I don't
//   recommend it.
//
//
// PERFORMANCE MEASUREMENTS FOR 1.06:
//
//                      32-bit     64-bit
//   Previous release:  8.83 s     7.68 s
//   Pool allocations:  7.72 s     6.34 s
//   Inline sort     :  6.54 s     5.65 s
//   New rasterizer  :  5.63 s     5.00 s

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////  SAMPLE PROGRAMS
////
//
//  Incomplete text-in-3d-api example, which draws quads properly aligned to be lossless
//
#if 0
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void my_stbtt_initfont(void)
{
   fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
   stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
   // can free ttf_buffer at this point
   glGenTextures(1, &ftex);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
   // can free temp_bitmap at this point
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void my_stbtt_print(float x, float y, char *text)
{
   // assume orthographic projection with units = screen pixels, origin at top left
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glBegin(GL_QUADS);
   while (*text) {
      if (*text >= 32 && *text < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y0);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y0);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y1);
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y1);
      }
      ++text;
   }
   glEnd();
}
#endif
//
//
//////////////////////////////////////////////////////////////////////////////
//
// Complete program (this compiles): get a single bitmap, print as ASCII art
//
#if 0
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

char ttf_buffer[1<<25];

int main(int argc, char **argv)
{
   stbtt_fontinfo font;
   unsigned char *bitmap;
   int w,h,i,j,c = (argc > 1 ? atoi(argv[1]) : 'a'), s = (argc > 2 ? atoi(argv[2]) : 20);

   fread(ttf_buffer, 1, 1<<25, fopen(argc > 3 ? argv[3] : "c:/windows/fonts/arialbd.ttf", "rb"));

   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
   bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), c, &w, &h, 0,0);

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i)
         putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
      putchar('\n');
   }
   return 0;
}
#endif
//
// Output:
//
//     .ii.
//    @@@@@@.
//   V@Mio@@o
//   :i.  V@V
//     :oM@@M
//   :@@@MM@M
//   @@o  o@M
//  :@@.  M@M
//   @@@o@@@@
//   :M@@V:@@.
//
//////////////////////////////////////////////////////////////////////////////
//
// Complete program: print "Hello World!" banner, with bugs
//
#if 0
char buffer[24<<20];
unsigned char screen[20][79];

int main(int arg, char **argv)
{
   stbtt_fontinfo font;
   int i,j,ascent,baseline,ch=0;
   float scale, xpos=2; // leave a little padding in case the character extends left
   char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

   fread(buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
   stbtt_InitFont(&font, buffer, 0);

   scale = stbtt_ScaleForPixelHeight(&font, 15);
   stbtt_GetFontVMetrics(&font, &ascent,0,0);
   baseline = (int) (ascent*scale);

   while (text[ch]) {
      int advance,lsb,x0,y0,x1,y1;
      float x_shift = xpos - (float) floor(xpos);
      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
      // because this API is really for baking character bitmaps into textures. if you want to render
      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
      // "alpha blend" that into the working buffer
      xpos += (advance * scale);
      if (text[ch+1])
         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
      ++ch;
   }

   for (j=0; j < 20; ++j) {
      for (i=0; i < 78; ++i)
         putchar(" .:ioVM@"[screen[j][i]>>5]);
      putchar('\n');
   }

   return 0;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////   INTEGRATION WITH YOUR CODEBASE
////
////   The following sections allow you to supply alternate definitions
////   of C library functions used by stb_truetype, e.g. if you don't
////   link with the C runtime library.

#ifdef STB_TRUETYPE_IMPLEMENTATION
   // #define your own (u)stbtt_int8/16/32 before including to override this
   #ifndef stbtt_uint8
   typedef unsigned char   stbtt_uint8;
   typedef signed   char   stbtt_int8;
   typedef unsigned short  stbtt_uint16;
   typedef signed   short  stbtt_int16;
   typedef unsigned int    stbtt_uint32;
   typedef signed   int    stbtt_int32;
   #endif

   typedef char stbtt__check_size32[sizeof(stbtt_int32)==4 ? 1 : -1];
   typedef char stbtt__check_size16[sizeof(stbtt_int16)==2 ? 1 : -1];

   // e.g. #define your own STBTT_ifloor/STBTT_iceil() to avoid math.h
   #ifndef STBTT_ifloor
   #include <math.h>
   #define STBTT_ifloor(x)   ((int) floor(x))
   #define STBTT_iceil(x)    ((int) ceil(x))
   #endif

   #ifndef STBTT_sqrt
   #include <math.h>
   #define STBTT_sqrt(x)      sqrt(x)
   #define STBTT_pow(x,y)     pow(x,y)
   #endif

   #ifndef STBTT_fmod
   #include <math.h>
   #define STBTT_fmod(x,y)    fmod(x,y)
   #endif

   #ifndef STBTT_cos
   #include <math.h>
   #define STBTT_cos(x)       cos(x)
   #define STBTT_acos(x)      acos(x)
   #endif

   #ifndef STBTT_fabs
   #include <math.h>
   #define STBTT_fabs(x)      fabs(x)
   #endif

   // #define your own functions "STBTT_malloc" / "STBTT_free" to avoid malloc.h
   #ifndef STBTT_malloc
   #include <stdlib.h>
   #define STBTT_malloc(x,u)  ((void)(u),malloc(x))
   #define STBTT_free(x,u)    ((void)(u),free(x))
   #endif

   #ifndef STBTT_assert
   #include <assert.h>
   #define STBTT_assert(x)    assert(x)
   #endif

   #ifndef STBTT_strlen
   #include <string.h>
   #define STBTT_strlen(x)    strlen(x)
   #endif

   #ifndef STBTT_memcpy
   #include <string.h>
   #define STBTT_memcpy       memcpy
   #define STBTT_memset       memset
   #endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   INTERFACE
////
////

#ifndef __STB_INCLUDE_STB_TRUETYPE_H__
#define __STB_INCLUDE_STB_TRUETYPE_H__

#ifdef STBTT_STATIC
#define STBTT_DEF static
#else
#define STBTT_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

// private structure
typedef struct
{
   unsigned char *data;
   int cursor;
   int size;
} stbtt__buf;

//////////////////////////////////////////////////////////////////////////////
//
// TEXTURE BAKING API
//
// If you use this API, you only have to call two functions ever.
//

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
} stbtt_bakedchar;

STBTT_DEF int stbtt_BakeFontBitmap(const unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                stbtt_bakedchar *chardata);             // you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

typedef struct
{
   float x0,y0,s0,t0; // top-left
   float x1,y1,s1,t1; // bottom-right
} stbtt_aligned_quad;

STBTT_DEF void stbtt_GetBakedQuad(const stbtt_bakedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               stbtt_aligned_quad *q,      // output: quad to draw
                               int opengl_fillrule);       // true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

STBTT_DEF void stbtt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap);
// Query the font vertical metrics without having to create a font first.


//////////////////////////////////////////////////////////////////////////////
//
// NEW TEXTURE BAKING API
//
// This provides options for packing multiple fonts into one atlas, not
// perfectly but better than nothing.

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
   float xoff2,yoff2;
} stbtt_packedchar;

typedef struct stbtt_pack_context stbtt_pack_context;
typedef struct stbtt_fontinfo stbtt_fontinfo;
#ifndef STB_RECT_PACK_VERSION
typedef struct stbrp_rect stbrp_rect;
#endif

STBTT_DEF int  stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context);
// Initializes a packing context stored in the passed-in stbtt_pack_context.
// Future calls using this context will pack characters into the bitmap passed
// in here: a 1-channel bitmap that is width * height. stride_in_bytes is
// the distance from one row to the next (or 0 to mean they are packed tightly
// together). "padding" is the amount of padding to leave between each
// character (normally you want '1' for bitmaps you'll use as textures with
// bilinear filtering).
//
// Returns 0 on failure, 1 on success.

STBTT_DEF void stbtt_PackEnd  (stbtt_pack_context *spc);
// Cleans up the packing context and frees all memory.

#define STBTT_POINT_SIZE(x)   (-(x))

STBTT_DEF int  stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
                                int first_unicode_char_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range);
// Creates character bitmaps from the font_index'th font found in fontdata (use
// font_index=0 if you don't know what that is). It creates num_chars_in_range
// bitmaps for characters with unicode values starting at first_unicode_char_in_range
// and increasing. Data for how to render them is stored in chardata_for_range;
// pass these to stbtt_GetPackedQuad to get back renderable quads.
//
// font_size is the full height of the character from ascender to descender,
// as computed by stbtt_ScaleForPixelHeight. To use a point size as computed
// by stbtt_ScaleForMappingEmToPixels, wrap the point size in STBTT_POINT_SIZE()
// and pass that result as 'font_size':
//       ...,                  20 , ... // font max minus min y is 20 pixels tall
//       ..., STBTT_POINT_SIZE(20), ... // 'M' is 20 pixels tall

typedef struct
{
   float font_size;
   int first_unicode_codepoint_in_range;  // if non-zero, then the chars are continuous, and this is the first codepoint
   int *array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
   int num_chars;
   stbtt_packedchar *chardata_for_range; // output
   unsigned char h_oversample, v_oversample; // don't set these, they're used internally
} stbtt_pack_range;

STBTT_DEF int  stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges);
// Creates character bitmaps from multiple ranges of characters stored in
// ranges. This will usually create a better-packed bitmap than multiple
// calls to stbtt_PackFontRange. Note that you can call this multiple
// times within a single PackBegin/PackEnd.

STBTT_DEF void stbtt_PackSetOversampling(stbtt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample);
// Oversampling a font increases the quality by allowing higher-quality subpixel
// positioning, and is especially valuable at smaller text sizes.
//
// This function sets the amount of oversampling for all following calls to
// stbtt_PackFontRange(s) or stbtt_PackFontRangesGatherRects for a given
// pack context. The default (no oversampling) is achieved by h_oversample=1
// and v_oversample=1. The total number of pixels required is
// h_oversample*v_oversample larger than the default; for example, 2x2
// oversampling requires 4x the storage of 1x1. For best results, render
// oversampled textures with bilinear filtering. Look at the readme in
// stb/tests/oversample for information about oversampled fonts
//
// To use with PackFontRangesGather etc., you must set it before calls
// call to PackFontRangesGatherRects.

STBTT_DEF void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context *spc, int skip);
// If skip != 0, this tells stb_truetype to skip any codepoints for which
// there is no corresponding glyph. If skip=0, which is the default, then
// codepoints without a glyph recived the font's "missing character" glyph,
// typically an empty box by convention.

STBTT_DEF void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               stbtt_aligned_quad *q,      // output: quad to draw
                               int align_to_integer);

STBTT_DEF int  stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects);
STBTT_DEF void stbtt_PackFontRangesPackRects(stbtt_pack_context *spc, stbrp_rect *rects, int num_rects);
STBTT_DEF int  stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects);
// Calling these functions in sequence is roughly equivalent to calling
// stbtt_PackFontRanges(). If you more control over the packing of multiple
// fonts, or if you want to pack custom data into a font texture, take a look
// at the source to of stbtt_PackFontRanges() and create a custom version
// using these functions, e.g. call GatherRects multiple times,
// building up a single array of rects, then call PackRects once,
// then call RenderIntoRects repeatedly. This may result in a
// better packing than calling PackFontRanges multiple times
// (or it may not).

// this is an opaque structure that you shouldn't mess with which holds
// all the context needed from PackBegin to PackEnd.
struct stbtt_pack_context {
   void *user_allocator_context;
   void *pack_info;
   int   width;
   int   height;
   int   stride_in_bytes;
   int   padding;
   int   skip_missing;
   unsigned int   h_oversample, v_oversample;
   unsigned char *pixels;
   void  *nodes;
};

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

STBTT_DEF int stbtt_GetNumberOfFonts(const unsigned char *data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index);
// Each .ttf/.ttc file may have more than one font. Each font has a sequential
// index number starting from 0. Call this function to get the font offset for
// a given index; it returns -1 if the index is out of range. A regular .ttf
// file will only define one font and it always be at offset 0, so it will
// return '0' for index 0, and -1 for all other indices.

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.
struct stbtt_fontinfo
{
   void           * userdata;
   unsigned char  * data;              // pointer to .ttf file
   int              fontstart;         // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern,gpos,svg; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph

   stbtt__buf cff;                    // cff font data
   stbtt__buf charstrings;            // the charstring index
   stbtt__buf gsubrs;                 // global charstring subroutines index
   stbtt__buf subrs;                  // private charstring subroutines index
   stbtt__buf fontdicts;              // array of font dicts
   stbtt__buf fdselect;               // map from glyph to fontdict
};

STBTT_DEF int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

STBTT_DEF int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

STBTT_DEF float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
// and computing:
//       scale = pixels / (ascent - descent)
// so if you prefer to measure height by the ascent only, use a similar calculation.

STBTT_DEF float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose EM size is mapped to
// 'pixels' tall. This is probably what traditional APIs compute, but
// I'm not positive.

STBTT_DEF void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
//   these are expressed in unscaled coordinates, so you must multiply by
//   the scale factor for a given size

STBTT_DEF int  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

STBTT_DEF void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1);
// the bounding box around all possible characters

STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

STBTT_DEF int  stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

STBTT_DEF int stbtt_GetCodepointBox(const stbtt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

STBTT_DEF void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing);
STBTT_DEF int  stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2);
STBTT_DEF int  stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
// as above, but takes one or more glyph indices for greater efficiency

typedef struct stbtt_kerningentry
{
   int glyph1; // use stbtt_FindGlyphIndex
   int glyph2;
   int advance;
} stbtt_kerningentry;

STBTT_DEF int  stbtt_GetKerningTableLength(const stbtt_fontinfo *info);
STBTT_DEF int  stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry* table, int table_length);
// Retrieves a complete list of all of the kerning pairs provided by the font
// stbtt_GetKerningTable never writes more than table_length entries and returns how many entries it did write.
// The table will be sorted by (a.glyph1 == b.glyph1)?(a.glyph2 < b.glyph2):(a.glyph1 < b.glyph1)

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

#ifndef STBTT_vmove // you can predefine these to use different values (but why?)
   enum {
      STBTT_vmove=1,
      STBTT_vline,
      STBTT_vcurve,
      STBTT_vcubic
   };
#endif

#ifndef stbtt_vertex // you can predefine this to use different values
                   // (we share this with other code at RAD)
   #define stbtt_vertex_type short // can't use stbtt_int16 because that's not visible in the header file
   typedef struct
   {
      stbtt_vertex_type x,y,cx,cy,cx1,cy1;
      unsigned char type,padding;
   } stbtt_vertex;
#endif

STBTT_DEF int stbtt_IsGlyphEmpty(const stbtt_fontinfo *info, int glyph_index);
// returns non-zero if nothing is drawn for this glyph

STBTT_DEF int stbtt_GetCodepointShape(const stbtt_fontinfo *info, int unicode_codepoint, stbtt_vertex **vertices);
STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **vertices);
// returns # of vertices and fills *vertices with the pointer to them
//   these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a STBTT_moveto, then consists of a series of mixed
// STBTT_lineto and STBTT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.

STBTT_DEF void stbtt_FreeShape(const stbtt_fontinfo *info, stbtt_vertex *vertices);
// frees the data allocated above

STBTT_DEF int stbtt_GetCodepointSVG(const stbtt_fontinfo *info, int unicode_codepoint, const char **svg);
STBTT_DEF int stbtt_GetGlyphSVG(const stbtt_fontinfo *info, int gl, const char **svg);
// fills svg with the character's SVG data.
// returns data size or 0 if SVG not found.

//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

STBTT_DEF void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata);
// frees the bitmap allocated below

STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

STBTT_DEF unsigned char *stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

STBTT_DEF void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
// the same as stbtt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call stbtt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint);
// same as stbtt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint);
// same as stbtt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see stbtt_PackSetOversampling)

STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

STBTT_DEF void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
STBTT_DEF void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int glyph);
STBTT_DEF void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
STBTT_DEF void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);


// @TODO: don't expose this structure
typedef struct
{
   int w,h,stride;
   unsigned char *pixels;
} stbtt__bitmap;

// rasterize a shape with quadratic beziers into a bitmap
STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result,        // 1-channel bitmap to draw into
                               float flatness_in_pixels,     // allowable error of curve in pixels
                               stbtt_vertex *vertices,       // array of vertices defining shape
                               int num_verts,                // number of vertices in above array
                               float scale_x, float scale_y, // scale applied to input vertices
                               float shift_x, float shift_y, // translation applied to input vertices
                               int x_off, int y_off,         // another translation applied to input
                               int invert,                   // if non-zero, vertically flip shape
                               void *userdata);              // context for to STBTT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

STBTT_DEF void stbtt_FreeSDF(unsigned char *bitmap, void *userdata);
// frees the SDF bitmap allocated below

STBTT_DEF unsigned char * stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char * stbtt_GetCodepointSDF(const stbtt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
// These functions compute a discretized SDF field for a single character, suitable for storing
// in a single-channel texture, sampling with bilinear filtering, and testing against
// larger than some threshold to produce scalable fonts.
//        info              --  the font
//        scale             --  controls the size of the resulting SDF bitmap, same as it would be creating a regular bitmap
//        glyph/codepoint   --  the character to generate the SDF for
//        padding           --  extra "pixels" around the character which are filled with the distance to the character (not 0),
//                                 which allows effects like bit outlines
//        onedge_value      --  value 0-255 to test the SDF against to reconstruct the character (i.e. the isocontour of the character)
//        pixel_dist_scale  --  what value the SDF should increase by when moving one SDF "pixel" away from the edge (on the 0..255 scale)
//                                 if positive, > onedge_value is inside; if negative, < onedge_value is inside
//        width,height      --  output height & width of the SDF bitmap (including padding)
//        xoff,yoff         --  output origin of the character
//        return value      --  a 2D array of bytes 0..255, width*height in size
//
// pixel_dist_scale & onedge_value are a scale & bias that allows you to make
// optimal use of the limited 0..255 for your application, trading off precision
// and special effects. SDF values outside the range 0..255 are clamped to 0..255.
//
// Example:
//      scale = stbtt_ScaleForPixelHeight(22)
//      padding = 5
//      onedge_value = 180
//      pixel_dist_scale = 180/5.0 = 36.0
//
//      This will create an SDF bitmap in which the character is about 22 pixels
//      high but the whole bitmap is about 22+5+5=32 pixels high. To produce a filled
//      shape, sample the SDF at each pixel and fill the pixel if the SDF value
//      is greater than or equal to 180/255. (You'll actually want to antialias,
//      which is beyond the scope of this example.) Additionally, you can compute
//      offset outlines (e.g. to stroke the character border inside & outside,
//      or only outside). For example, to fill outside the character up to 3 SDF
//      pixels, you would compare against (180-36.0*3)/255 = 72/255. The above
//      choice of variables maps a range from 5 pixels outside the shape to
//      2 pixels inside the shape to 0..255; this is intended primarily for apply
//      outside effects only (the interior range is needed to allow proper
//      antialiasing of the font at *smaller* sizes)
//
// The function computes the SDF analytically at each SDF pixel, not by e.g.
// building a higher-res bitmap and approximating it. In theory the quality
// should be as high as possible for an SDF of this size & representation, but
// unclear if this is true in practice (perhaps building a higher-res bitmap
// and computing from that can allow drop-out prevention).
//
// The algorithm has not been optimized at all, so expect it to be slow
// if computing lots of characters or very large sizes.



//////////////////////////////////////////////////////////////////////////////
//
// Finding the right font...
//
// You should really just solve this offline, keep your own tables
// of what font is what, and don't try to get it out of the .ttf file.
// That's because getting it out of the .ttf file is really hard, because
// the names in the file can appear in many possible encodings, in many
// possible languages, and e.g. if you need a case-insensitive comparison,
// the details of that depend on the encoding & language in a complex way
// (actually underspecified in truetype, but also gigantic).
//
// But you can use the provided functions in two possible ways:
//     stbtt_FindMatchingFont() will use *case-sensitive* comparisons on
//             unicode-encoded names to try to find the font you want;
//             you can run this before calling stbtt_InitFont()
//
//     stbtt_GetFontNameString() lets you get any of the various strings
//             from the file yourself and do your own comparisons on them.
//             You have to have called stbtt_InitFont() first.


STBTT_DEF int stbtt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use STBTT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define STBTT_MACSTYLE_DONTCARE     0
#define STBTT_MACSTYLE_BOLD         1
#define STBTT_MACSTYLE_ITALIC       2
#define STBTT_MACSTYLE_UNDERSCORE   4
#define STBTT_MACSTYLE_NONE         8   // <= not same as 0, this makes us check the bitfield is 0

STBTT_DEF int stbtt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2);
// returns 1/0 whether the first string interpreted as utf8 is identical to
// the second string interpreted as big-endian utf16... useful for strings from next func

STBTT_DEF const char *stbtt_GetFontNameString(const stbtt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID);
// returns the string (which may be big-endian double byte, e.g. for unicode)
// and puts the length in bytes in *length.
//
// some of the values for the IDs are below; for more see the truetype spec:
//     http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6name.html
//     http://www.microsoft.com/typography/otspec/name.htm

enum { // platformID
   STBTT_PLATFORM_ID_UNICODE   =0,
   STBTT_PLATFORM_ID_MAC       =1,
   STBTT_PLATFORM_ID_ISO       =2,
   STBTT_PLATFORM_ID_MICROSOFT =3
};

enum { // encodingID for STBTT_PLATFORM_ID_UNICODE
   STBTT_UNICODE_EID_UNICODE_1_0    =0,
   STBTT_UNICODE_EID_UNICODE_1_1    =1,
   STBTT_UNICODE_EID_ISO_10646      =2,
   STBTT_UNICODE_EID_UNICODE_2_0_BMP=3,
   STBTT_UNICODE_EID_UNICODE_2_0_FULL=4
};

enum { // encodingID for STBTT_PLATFORM_ID_MICROSOFT
   STBTT_MS_EID_SYMBOL        =0,
   STBTT_MS_EID_UNICODE_BMP   =1,
   STBTT_MS_EID_SHIFTJIS      =2,
   STBTT_MS_EID_UNICODE_FULL  =10
};

enum { // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
   STBTT_MAC_EID_ROMAN        =0,   STBTT_MAC_EID_ARABIC       =4,
   STBTT_MAC_EID_JAPANESE     =1,   STBTT_MAC_EID_HEBREW       =5,
   STBTT_MAC_EID_CHINESE_TRAD =2,   STBTT_MAC_EID_GREEK        =6,
   STBTT_MAC_EID_KOREAN       =3,   STBTT_MAC_EID_RUSSIAN      =7
};

enum { // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
       // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
   STBTT_MS_LANG_ENGLISH     =0x0409,   STBTT_MS_LANG_ITALIAN     =0x0410,
   STBTT_MS_LANG_CHINESE     =0x0804,   STBTT_MS_LANG_JAPANESE    =0x0411,
   STBTT_MS_LANG_DUTCH       =0x0413,   STBTT_MS_LANG_KOREAN      =0x0412,
   STBTT_MS_LANG_FRENCH      =0x040c,   STBTT_MS_LANG_RUSSIAN     =0x0419,
   STBTT_MS_LANG_GERMAN      =0x0407,   STBTT_MS_LANG_SPANISH     =0x0409,
   STBTT_MS_LANG_HEBREW      =0x040d,   STBTT_MS_LANG_SWEDISH     =0x041D
};

enum { // languageID for STBTT_PLATFORM_ID_MAC
   STBTT_MAC_LANG_ENGLISH      =0 ,   STBTT_MAC_LANG_JAPANESE     =11,
   STBTT_MAC_LANG_ARABIC       =12,   STBTT_MAC_LANG_KOREAN       =23,
   STBTT_MAC_LANG_DUTCH        =4 ,   STBTT_MAC_LANG_RUSSIAN      =32,
   STBTT_MAC_LANG_FRENCH       =1 ,   STBTT_MAC_LANG_SPANISH      =6 ,
   STBTT_MAC_LANG_GERMAN       =2 ,   STBTT_MAC_LANG_SWEDISH      =5 ,
   STBTT_MAC_LANG_HEBREW       =10,   STBTT_MAC_LANG_CHINESE_SIMPLIFIED =33,
   STBTT_MAC_LANG_ITALIAN      =3 ,   STBTT_MAC_LANG_CHINESE_TRAD =19
};

#ifdef __cplusplus
}
#endif

#endif // __STB_INCLUDE_STB_TRUETYPE_H__

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   IMPLEMENTATION
////
////

#ifdef STB_TRUETYPE_IMPLEMENTATION

#ifndef STBTT_MAX_OVERSAMPLE
#define STBTT_MAX_OVERSAMPLE   8
#endif

#if STBTT_MAX_OVERSAMPLE > 255
#error "STBTT_MAX_OVERSAMPLE cannot be > 255"
#endif

typedef int stbtt__test_oversample_pow2[(STBTT_MAX_OVERSAMPLE & (STBTT_MAX_OVERSAMPLE-1)) == 0 ? 1 : -1];

#ifndef STBTT_RASTERIZER_VERSION
#define STBTT_RASTERIZER_VERSION 2
#endif

#ifdef _MSC_VER
#define STBTT__NOTUSED(v)  (void)(v)
#else
#define STBTT__NOTUSED(v)  (void)sizeof(v)
#endif

//////////////////////////////////////////////////////////////////////////
//
// stbtt__buf helpers to parse data from file
//

static stbtt_uint8 stbtt__buf_get8(stbtt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor++];
}

static stbtt_uint8 stbtt__buf_peek8(stbtt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor];
}

static void stbtt__buf_seek(stbtt__buf *b, int o)
{
   STBTT_assert(!(o > b->size || o < 0));
   b->cursor = (o > b->size || o < 0) ? b->size : o;
}

static void stbtt__buf_skip(stbtt__buf *b, int o)
{
   stbtt__buf_seek(b, b->cursor + o);
}

static stbtt_uint32 stbtt__buf_get(stbtt__buf *b, int n)
{
   stbtt_uint32 v = 0;
   int i;
   STBTT_assert(n >= 1 && n <= 4);
   for (i = 0; i < n; i++)
      v = (v << 8) | stbtt__buf_get8(b);
   return v;
}

static stbtt__buf stbtt__new_buf(const void *p, size_t size)
{
   stbtt__buf r;
   STBTT_assert(size < 0x40000000);
   r.data = (stbtt_uint8*) p;
   r.size = (int) size;
   r.cursor = 0;
   return r;
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)

static stbtt__buf stbtt__buf_range(const stbtt__buf *b, int o, int s)
{
   stbtt__buf r = stbtt__new_buf(NULL, 0);
   if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
   r.data = b->data + o;
   r.size = s;
   return r;
}

static stbtt__buf stbtt__cff_get_index(stbtt__buf *b)
{
   int count, start, offsize;
   start = b->cursor;
   count = stbtt__buf_get16(b);
   if (count) {
      offsize = stbtt__buf_get8(b);
      STBTT_assert(offsize >= 1 && offsize <= 4);
      stbtt__buf_skip(b, offsize * count);
      stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
   }
   return stbtt__buf_range(b, start, b->cursor - start);
}

static stbtt_uint32 stbtt__cff_int(stbtt__buf *b)
{
   int b0 = stbtt__buf_get8(b);
   if (b0 >= 32 && b0 <= 246)       return b0 - 139;
   else if (b0 >= 247 && b0 <= 250) return (b0 - 247)*256 + stbtt__buf_get8(b) + 108;
   else if (b0 >= 251 && b0 <= 254) return -(b0 - 251)*256 - stbtt__buf_get8(b) - 108;
   else if (b0 == 28)               return stbtt__buf_get16(b);
   else if (b0 == 29)               return stbtt__buf_get32(b);
   STBTT_assert(0);
   return 0;
}

static void stbtt__cff_skip_operand(stbtt__buf *b) {
   int v, b0 = stbtt__buf_peek8(b);
   STBTT_assert(b0 >= 28);
   if (b0 == 30) {
      stbtt__buf_skip(b, 1);
      while (b->cursor < b->size) {
         v = stbtt__buf_get8(b);
         if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
            break;
      }
   } else {
      stbtt__cff_int(b);
   }
}

static stbtt__buf stbtt__dict_get(stbtt__buf *b, int key)
{
   stbtt__buf_seek(b, 0);
   while (b->cursor < b->size) {
      int start = b->cursor, end, op;
      while (stbtt__buf_peek8(b) >= 28)
         stbtt__cff_skip_operand(b);
      end = b->cursor;
      op = stbtt__buf_get8(b);
      if (op == 12)  op = stbtt__buf_get8(b) | 0x100;
      if (op == key) return stbtt__buf_range(b, start, end-start);
   }
   return stbtt__buf_range(b, 0, 0);
}

static void stbtt__dict_get_ints(stbtt__buf *b, int key, int outcount, stbtt_uint32 *out)
{
   int i;
   stbtt__buf operands = stbtt__dict_get(b, key);
   for (i = 0; i < outcount && operands.cursor < operands.size; i++)
      out[i] = stbtt__cff_int(&operands);
}

static int stbtt__cff_index_count(stbtt__buf *b)
{
   stbtt__buf_seek(b, 0);
   return stbtt__buf_get16(b);
}

static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i)
{
   int count, offsize, start, end;
   stbtt__buf_seek(&b, 0);
   count = stbtt__buf_get16(&b);
   offsize = stbtt__buf_get8(&b);
   STBTT_assert(i >= 0 && i < count);
   STBTT_assert(offsize >= 1 && offsize <= 4);
   stbtt__buf_skip(&b, i*offsize);
   start = stbtt__buf_get(&b, offsize);
   end = stbtt__buf_get(&b, offsize);
   return stbtt__buf_range(&b, 2+(count+1)*offsize+start, end - start);
}

//////////////////////////////////////////////////////////////////////////
//
// accessors to parse data from file
//

// on platforms that don't allow misaligned reads, if we want to allow
// truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

#define ttBYTE(p)     (* (stbtt_uint8 *) (p))
#define ttCHAR(p)     (* (stbtt_int8 *) (p))
#define ttFixed(p)    ttLONG(p)

static stbtt_uint16 ttUSHORT(stbtt_uint8 *p) { return p[0]*256 + p[1]; }
static stbtt_int16 ttSHORT(stbtt_uint8 *p)   { return p[0]*256 + p[1]; }
static stbtt_uint32 ttULONG(stbtt_uint8 *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
static stbtt_int32 ttLONG(stbtt_uint8 *p)    { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }

#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)           stbtt_tag4(p,str[0],str[1],str[2],str[3])

static int stbtt__isfont(stbtt_uint8 *font)
{
   // check the version number
   if (stbtt_tag4(font, '1',0,0,0))  return 1; // TrueType 1
   if (stbtt_tag(font, "typ1"))   return 1; // TrueType with type 1 font -- we don't support this!
   if (stbtt_tag(font, "OTTO"))   return 1; // OpenType with CFF
   if (stbtt_tag4(font, 0,1,0,0)) return 1; // OpenType 1.0
   if (stbtt_tag(font, "true"))   return 1; // Apple specification for TrueType fonts
   return 0;
}

// @OPTIMIZE: binary search
static stbtt_uint32 stbtt__find_table(stbtt_uint8 *data, stbtt_uint32 fontstart, const char *tag)
{
   stbtt_int32 num_tables = ttUSHORT(data+fontstart+4);
   stbtt_uint32 tabledir = fontstart + 12;
   stbtt_int32 i;
   for (i=0; i < num_tables; ++i) {
      stbtt_uint32 loc = tabledir + 16*i;
      if (stbtt_tag(data+loc+0, tag))
         return ttULONG(data+loc+8);
   }
   return 0;
}

static int stbtt_GetFontOffsetForIndex_internal(unsigned char *font_collection, int index)
{
   // if it's just a font, there's only one valid index
   if (stbtt__isfont(font_collection))
      return index == 0 ? 0 : -1;

   // check if it's a TTC
   if (stbtt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         stbtt_int32 n = ttLONG(font_collection+8);
         if (index >= n)
            return -1;
         return ttULONG(font_collection+12+index*4);
      }
   }
   return -1;
}

static int stbtt_GetNumberOfFonts_internal(unsigned char *font_collection)
{
   // if it's just a font, there's only one valid font
   if (stbtt__isfont(font_collection))
      return 1;

   // check if it's a TTC
   if (stbtt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         return ttLONG(font_collection+8);
      }
   }
   return 0;
}

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
   stbtt_uint32 subrsoff = 0, private_loc[2] = { 0, 0 };
   stbtt__buf pdict;
   stbtt__dict_get_ints(&fontdict, 18, 2, private_loc);
   if (!private_loc[1] || !private_loc[0]) return stbtt__new_buf(NULL, 0);
   pdict = stbtt__buf_range(&cff, private_loc[1], private_loc[0]);
   stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);
   if (!subrsoff) return stbtt__new_buf(NULL, 0);
   stbtt__buf_seek(&cff, private_loc[1]+subrsoff);
   return stbtt__cff_get_index(&cff);
}

// since most people won't use this, find this table the first time it's needed
static int stbtt__get_svg(stbtt_fontinfo *info)
{
   stbtt_uint32 t;
   if (info->svg < 0) {
      t = stbtt__find_table(info->data, info->fontstart, "SVG ");
      if (t) {
         stbtt_uint32 offset = ttULONG(info->data + t + 2);
         info->svg = t + offset;
      } else {
         info->svg = 0;
      }
   }
   return info->svg;
}

static int stbtt_InitFont_internal(stbtt_fontinfo *info, unsigned char *data, int fontstart)
{
   stbtt_uint32 cmap, t;
   stbtt_int32 i,numTables;

   info->data = data;
   info->fontstart = fontstart;
   info->cff = stbtt__new_buf(NULL, 0);

   cmap = stbtt__find_table(data, fontstart, "cmap");       // required
   info->loca = stbtt__find_table(data, fontstart, "loca"); // required
   info->head = stbtt__find_table(data, fontstart, "head"); // required
   info->glyf = stbtt__find_table(data, fontstart, "glyf"); // required
   info->hhea = stbtt__find_table(data, fontstart, "hhea"); // required
   info->hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
   info->kern = stbtt__find_table(data, fontstart, "kern"); // not required
   info->gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required

   if (!cmap || !info->head || !info->hhea || !info->hmtx)
      return 0;
   if (info->glyf) {
      // required for truetype
      if (!info->loca) return 0;
   } else {
      // initialization for CFF / Type2 fonts (OTF)
      stbtt__buf b, topdict, topdictidx;
      stbtt_uint32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
      stbtt_uint32 cff;

      cff = stbtt__find_table(data, fontstart, "CFF ");
      if (!cff) return 0;

      info->fontdicts = stbtt__new_buf(NULL, 0);
      info->fdselect = stbtt__new_buf(NULL, 0);

      // @TODO this should use size from table (not 512MB)
      info->cff = stbtt__new_buf(data+cff, 512*1024*1024);
      b = info->cff;

      // read the header
      stbtt__buf_skip(&b, 2);
      stbtt__buf_seek(&b, stbtt__buf_get8(&b)); // hdrsize

      // @TODO the name INDEX could list multiple fonts,
      // but we just use the first one.
      stbtt__cff_get_index(&b);  // name INDEX
      topdictidx = stbtt__cff_get_index(&b);
      topdict = stbtt__cff_index_get(topdictidx, 0);
      stbtt__cff_get_index(&b);  // string INDEX
      info->gsubrs = stbtt__cff_get_index(&b);

      stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
      stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
      stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
      stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
      info->subrs = stbtt__get_subrs(b, topdict);

      // we only support Type 2 charstrings
      if (cstype != 2) return 0;
      if (charstrings == 0) return 0;

      if (fdarrayoff) {
         // looks like a CID font
         if (!fdselectoff) return 0;
         stbtt__buf_seek(&b, fdarrayoff);
         info->fontdicts = stbtt__cff_get_index(&b);
         info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size-fdselectoff);
      }

      stbtt__buf_seek(&b, charstrings);
      info->charstrings = stbtt__cff_get_index(&b);
   }

   t = stbtt__find_table(data, fontstart, "maxp");
   if (t)
      info->numGlyphs = ttUSHORT(data+t+4);
   else
      info->numGlyphs = 0xffff;

   info->svg = -1;

   // find a cmap encoding table we understand *now* to avoid searching
   // later. (todo: could make this installable)
   // the same regardless of glyph.
   numTables = ttUSHORT(data + cmap + 2);
   info->index_map = 0;
   for (i=0; i < numTables; ++i) {
      stbtt_uint32 encoding_record = cmap + 4 + 8 * i;
      // find an encoding we understand:
      switch(ttUSHORT(data+encoding_record)) {
         case STBTT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(data+encoding_record+2)) {
               case STBTT_MS_EID_UNICODE_BMP:
               case STBTT_MS_EID_UNICODE_FULL:
                  // MS/Unicode
                  info->index_map = cmap + ttULONG(data+encoding_record+4);
                  break;
            }
            break;
        case STBTT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->index_map = cmap + ttULONG(data+encoding_record+4);
            break;
      }
   }
   if (info->index_map == 0)
      return 0;

   info->indexToLocFormat = ttUSHORT(data+info->head + 50);
   return 1;
}

STBTT_DEF int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint)
{
   stbtt_uint8 *data = info->data;
   stbtt_uint32 index_map = info->index_map;

   stbtt_uint16 format = ttUSHORT(data + index_map + 0);
   if (format == 0) { // apple byte encoding
      stbtt_int32 bytes = ttUSHORT(data + index_map + 2);
      if (unicode_codepoint < bytes-6)
         return ttBYTE(data + index_map + 6 + unicode_codepoint);
      return 0;
   } else if (format == 6) {
      stbtt_uint32 first = ttUSHORT(data + index_map + 6);
      stbtt_uint32 count = ttUSHORT(data + index_map + 8);
      if ((stbtt_uint32) unicode_codepoint >= first && (stbtt_uint32) unicode_codepoint < first+count)
         return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
      return 0;
   } else if (format == 2) {
      STBTT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
      return 0;
   } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
      stbtt_uint16 segcount = ttUSHORT(data+index_map+6) >> 1;
      stbtt_uint16 searchRange = ttUSHORT(data+index_map+8) >> 1;
      stbtt_uint16 entrySelector = ttUSHORT(data+index_map+10);
      stbtt_uint16 rangeShift = ttUSHORT(data+index_map+12) >> 1;

      // do a binary search of the segments
      stbtt_uint32 endCount = index_map + 14;
      stbtt_uint32 search = endCount;

      if (unicode_codepoint > 0xffff)
         return 0;

      // they lie from endCount .. endCount + segCount
      // but searchRange is the nearest power of two, so...
      if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2))
         search += rangeShift*2;

      // now decrement to bias correctly to find smallest
      search -= 2;
      while (entrySelector) {
         stbtt_uint16 end;
         searchRange >>= 1;
         end = ttUSHORT(data + search + searchRange*2);
         if (unicode_codepoint > end)
            search += searchRange*2;
         --entrySelector;
      }
      search += 2;

      {
         stbtt_uint16 offset, start;
         stbtt_uint16 item = (stbtt_uint16) ((search - endCount) >> 1);

         STBTT_assert(unicode_codepoint <= ttUSHORT(data + endCount + 2*item));
         start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
         if (unicode_codepoint < start)
            return 0;

         offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
         if (offset == 0)
            return (stbtt_uint16) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));

         return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
      }
   } else if (format == 12 || format == 13) {
      stbtt_uint32 ngroups = ttULONG(data+index_map+12);
      stbtt_int32 low,high;
      low = 0; high = (stbtt_int32)ngroups;
      // Binary search the right group.
      while (low < high) {
         stbtt_int32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
         stbtt_uint32 start_char = ttULONG(data+index_map+16+mid*12);
         stbtt_uint32 end_char = ttULONG(data+index_map+16+mid*12+4);
         if ((stbtt_uint32) unicode_codepoint < start_char)
            high = mid;
         else if ((stbtt_uint32) unicode_codepoint > end_char)
            low = mid+1;
         else {
            stbtt_uint32 start_glyph = ttULONG(data+index_map+16+mid*12+8);
            if (format == 12)
               return start_glyph + unicode_codepoint-start_char;
            else // format == 13
               return start_glyph;
         }
      }
      return 0; // not found
   }
   // @TODO
   STBTT_assert(0);
   return 0;
}

STBTT_DEF int stbtt_GetCodepointShape(const stbtt_fontinfo *info, int unicode_codepoint, stbtt_vertex **vertices)
{
   return stbtt_GetGlyphShape(info, stbtt_FindGlyphIndex(info, unicode_codepoint), vertices);
}

static void stbtt_setvertex(stbtt_vertex *v, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy)
{
   v->type = type;
   v->x = (stbtt_int16) x;
   v->y = (stbtt_int16) y;
   v->cx = (stbtt_int16) cx;
   v->cy = (stbtt_int16) cy;
}

static int stbtt__GetGlyfOffset(const stbtt_fontinfo *info, int glyph_index)
{
   int g1,g2;

   STBTT_assert(!info->cff.size);

   if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
   if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

   if (info->indexToLocFormat == 0) {
      g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
      g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
   } else {
      g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
      g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
   }

   return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);

STBTT_DEF int stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   if (info->cff.size) {
      stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
   } else {
      int g = stbtt__GetGlyfOffset(info, glyph_index);
      if (g < 0) return 0;

      if (x0) *x0 = ttSHORT(info->data + g + 2);
      if (y0) *y0 = ttSHORT(info->data + g + 4);
      if (x1) *x1 = ttSHORT(info->data + g + 6);
      if (y1) *y1 = ttSHORT(info->data + g + 8);
   }
   return 1;
}

STBTT_DEF int stbtt_GetCodepointBox(const stbtt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1)
{
   return stbtt_GetGlyphBox(info, stbtt_FindGlyphIndex(info,codepoint), x0,y0,x1,y1);
}

STBTT_DEF int stbtt_IsGlyphEmpty(const stbtt_fontinfo *info, int glyph_index)
{
   stbtt_int16 numberOfContours;
   int g;
   if (info->cff.size)
      return stbtt__GetGlyphInfoT2(info, glyph_index, NULL, NULL, NULL, NULL) == 0;
   g = stbtt__GetGlyfOffset(info, glyph_index);
   if (g < 0) return 1;
   numberOfContours = ttSHORT(info->data + g);
   return numberOfContours == 0;
}

static int stbtt__close_shape(stbtt_vertex *vertices, int num_vertices, int was_off, int start_off,
    stbtt_int32 sx, stbtt_int32 sy, stbtt_int32 scx, stbtt_int32 scy, stbtt_int32 cx, stbtt_int32 cy)
{
   if (start_off) {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
      stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx,sy,scx,scy);
   } else {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve,sx,sy,cx,cy);
      else
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vline,sx,sy,0,0);
   }
   return num_vertices;
}

static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   stbtt_int16 numberOfContours;
   stbtt_uint8 *endPtsOfContours;
   stbtt_uint8 *data = info->data;
   stbtt_vertex *vertices=0;
   int num_vertices=0;
   int g = stbtt__GetGlyfOffset(info, glyph_index);

   *pvertices = NULL;

   if (g < 0) return 0;

   numberOfContours = ttSHORT(data + g);

   if (numberOfContours > 0) {
      stbtt_uint8 flags=0,flagcount;
      stbtt_int32 ins, i,j=0,m,n, next_move, was_off=0, off, start_off=0;
      stbtt_int32 x,y,cx,cy,sx,sy, scx,scy;
      stbtt_uint8 *points;
      endPtsOfContours = (data + g + 10);
      ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
      points = data + g + 10 + numberOfContours * 2 + 2 + ins;

      n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);

      m = n + 2*numberOfContours;  // a loose bound on how many vertices we might need
      vertices = (stbtt_vertex *) STBTT_malloc(m * sizeof(vertices[0]), info->userdata);
      if (vertices == 0)
         return 0;

      next_move = 0;
      flagcount=0;

      // in first pass, we load uninterpreted data into the allocated array
      // above, shifted to the end of the array so we won't overwrite it when
      // we create our final data starting from the front

      off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated

      // first load flags

      for (i=0; i < n; ++i) {
         if (flagcount == 0) {
            flags = *points++;
            if (flags & 8)
               flagcount = *points++;
         } else
            --flagcount;
         vertices[off+i].type = flags;
      }

      // now load x coordinates
      x=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 2) {
            stbtt_int16 dx = *points++;
            x += (flags & 16) ? dx : -dx; // ???
         } else {
            if (!(flags & 16)) {
               x = x + (stbtt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].x = (stbtt_int16) x;
      }

      // now load y coordinates
      y=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 4) {
            stbtt_int16 dy = *points++;
            y += (flags & 32) ? dy : -dy; // ???
         } else {
            if (!(flags & 32)) {
               y = y + (stbtt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].y = (stbtt_int16) y;
      }

      // now convert them to our format
      num_vertices=0;
      sx = sy = cx = cy = scx = scy = 0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         x     = (stbtt_int16) vertices[off+i].x;
         y     = (stbtt_int16) vertices[off+i].y;

         if (next_move == i) {
            if (i != 0)
               num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);

            // now start the new one
            start_off = !(flags & 1);
            if (start_off) {
               // if we start off with an off-curve point, then when we need to find a point on the curve
               // where we can start, and we need to save some state for when we wraparound.
               scx = x;
               scy = y;
               if (!(vertices[off+i+1].type & 1)) {
                  // next point is also a curve point, so interpolate an on-point curve
                  sx = (x + (stbtt_int32) vertices[off+i+1].x) >> 1;
                  sy = (y + (stbtt_int32) vertices[off+i+1].y) >> 1;
               } else {
                  // otherwise just use the next point as our start point
                  sx = (stbtt_int32) vertices[off+i+1].x;
                  sy = (stbtt_int32) vertices[off+i+1].y;
                  ++i; // we're using point i+1 as the starting point, so skip it
               }
            } else {
               sx = x;
               sy = y;
            }
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove,sx,sy,0,0);
            was_off = 0;
            next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
            ++j;
         } else {
            if (!(flags & 1)) { // if it's a curve
               if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
               cx = x;
               cy = y;
               was_off = 1;
            } else {
               if (was_off)
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x,y, cx, cy);
               else
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x,y,0,0);
               was_off = 0;
            }
         }
      }
      num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
   } else if (numberOfContours < 0) {
      // Compound shapes.
      int more = 1;
      stbtt_uint8 *comp = data + g + 10;
      num_vertices = 0;
      vertices = 0;
      while (more) {
         stbtt_uint16 flags, gidx;
         int comp_num_verts = 0, i;
         stbtt_vertex *comp_verts = 0, *tmp = 0;
         float mtx[6] = {1,0,0,1,0,0}, m, n;

         flags = ttSHORT(comp); comp+=2;
         gidx = ttSHORT(comp); comp+=2;

         if (flags & 2) { // XY values
            if (flags & 1) { // shorts
               mtx[4] = ttSHORT(comp); comp+=2;
               mtx[5] = ttSHORT(comp); comp+=2;
            } else {
               mtx[4] = ttCHAR(comp); comp+=1;
               mtx[5] = ttCHAR(comp); comp+=1;
            }
         }
         else {
            // @TODO handle matching point
            STBTT_assert(0);
         }
         if (flags & (1<<3)) { // WE_HAVE_A_SCALE
            mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
         } else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         } else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         }

         // Find transformation scales.
         m = (float) STBTT_sqrt(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
         n = (float) STBTT_sqrt(mtx[2]*mtx[2] + mtx[3]*mtx[3]);

         // Get indexed glyph.
         comp_num_verts = stbtt_GetGlyphShape(info, gidx, &comp_verts);
         if (comp_num_verts > 0) {
            // Transform vertices.
            for (i = 0; i < comp_num_verts; ++i) {
               stbtt_vertex* v = &comp_verts[i];
               stbtt_vertex_type x,y;
               x=v->x; y=v->y;
               v->x = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->y = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
               x=v->cx; y=v->cy;
               v->cx = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->cy = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
            }
            // Append vertices.
            tmp = (stbtt_vertex*)STBTT_malloc((num_vertices+comp_num_verts)*sizeof(stbtt_vertex), info->userdata);
            if (!tmp) {
               if (vertices) STBTT_free(vertices, info->userdata);
               if (comp_verts) STBTT_free(comp_verts, info->userdata);
               return 0;
            }
            if (num_vertices > 0) STBTT_memcpy(tmp, vertices, num_vertices*sizeof(stbtt_vertex));
            STBTT_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(stbtt_vertex));
            if (vertices) STBTT_free(vertices, info->userdata);
            vertices = tmp;
            STBTT_free(comp_verts, info->userdata);
            num_vertices += comp_num_verts;
         }
         // More components ?
         more = flags & (1<<5);
      }
   } else {
      // numberOfCounters == 0, do nothing
   }

   *pvertices = vertices;
   return num_vertices;
}

typedef struct
{
   int bounds;
   int started;
   float first_x, first_y;
   float x, y;
   stbtt_int32 min_x, max_x, min_y, max_y;

   stbtt_vertex *pvertices;
   int num_vertices;
} stbtt__csctx;

#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}

static void stbtt__track_vertex(stbtt__csctx *c, stbtt_int32 x, stbtt_int32 y)
{
   if (x > c->max_x || !c->started) c->max_x = x;
   if (y > c->max_y || !c->started) c->max_y = y;
   if (x < c->min_x || !c->started) c->min_x = x;
   if (y < c->min_y || !c->started) c->min_y = y;
   c->started = 1;
}

static void stbtt__csctx_v(stbtt__csctx *c, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy, stbtt_int32 cx1, stbtt_int32 cy1)
{
   if (c->bounds) {
      stbtt__track_vertex(c, x, y);
      if (type == STBTT_vcubic) {
         stbtt__track_vertex(c, cx, cy);
         stbtt__track_vertex(c, cx1, cy1);
      }
   } else {
      stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
      c->pvertices[c->num_vertices].cx1 = (stbtt_int16) cx1;
      c->pvertices[c->num_vertices].cy1 = (stbtt_int16) cy1;
   }
   c->num_vertices++;
}

static void stbtt__csctx_close_shape(stbtt__csctx *ctx)
{
   if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
      stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void stbtt__csctx_rmove_to(stbtt__csctx *ctx, float dx, float dy)
{
   stbtt__csctx_close_shape(ctx);
   ctx->first_x = ctx->x = ctx->x + dx;
   ctx->first_y = ctx->y = ctx->y + dy;
   stbtt__csctx_v(ctx, STBTT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rline_to(stbtt__csctx *ctx, float dx, float dy)
{
   ctx->x += dx;
   ctx->y += dy;
   stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rccurve_to(stbtt__csctx *ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
   float cx1 = ctx->x + dx1;
   float cy1 = ctx->y + dy1;
   float cx2 = cx1 + dx2;
   float cy2 = cy1 + dy2;
   ctx->x = cx2 + dx3;
   ctx->y = cy2 + dy3;
   stbtt__csctx_v(ctx, STBTT_vcubic, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static stbtt__buf stbtt__get_subr(stbtt__buf idx, int n)
{
   int count = stbtt__cff_index_count(&idx);
   int bias = 107;
   if (count >= 33900)
      bias = 32768;
   else if (count >= 1240)
      bias = 1131;
   n += bias;
   if (n < 0 || n >= count)
      return stbtt__new_buf(NULL, 0);
   return stbtt__cff_index_get(idx, n);
}

static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo *info, int glyph_index)
{
   stbtt__buf fdselect = info->fdselect;
   int nranges, start, end, v, fmt, fdselector = -1, i;

   stbtt__buf_seek(&fdselect, 0);
   fmt = stbtt__buf_get8(&fdselect);
   if (fmt == 0) {
      // untested
      stbtt__buf_skip(&fdselect, glyph_index);
      fdselector = stbtt__buf_get8(&fdselect);
   } else if (fmt == 3) {
      nranges = stbtt__buf_get16(&fdselect);
      start = stbtt__buf_get16(&fdselect);
      for (i = 0; i < nranges; i++) {
         v = stbtt__buf_get8(&fdselect);
         end = stbtt__buf_get16(&fdselect);
         if (glyph_index >= start && glyph_index < end) {
            fdselector = v;
            break;
         }
         start = end;
      }
   }
   if (fdselector == -1) stbtt__new_buf(NULL, 0);
   return stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
}

static int stbtt__run_charstring(const stbtt_fontinfo *info, int glyph_index, stbtt__csctx *c)
{
   int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
   int has_subrs = 0, clear_stack;
   float s[48];
   stbtt__buf subr_stack[10], subrs = info->subrs, b;
   float f;

#define STBTT__CSERR(s) (0)

   // this currently ignores the initial width value, which isn't needed if we have hmtx
   b = stbtt__cff_index_get(info->charstrings, glyph_index);
   while (b.cursor < b.size) {
      i = 0;
      clear_stack = 1;
      b0 = stbtt__buf_get8(&b);
      switch (b0) {
      // @TODO implement hinting
      case 0x13: // hintmask
      case 0x14: // cntrmask
         if (in_header)
            maskbits += (sp / 2); // implicit "vstem"
         in_header = 0;
         stbtt__buf_skip(&b, (maskbits + 7) / 8);
         break;

      case 0x01: // hstem
      case 0x03: // vstem
      case 0x12: // hstemhm
      case 0x17: // vstemhm
         maskbits += (sp / 2);
         break;

      case 0x15: // rmoveto
         in_header = 0;
         if (sp < 2) return STBTT__CSERR("rmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-2], s[sp-1]);
         break;
      case 0x04: // vmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("vmoveto stack");
         stbtt__csctx_rmove_to(c, 0, s[sp-1]);
         break;
      case 0x16: // hmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("hmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-1], 0);
         break;

      case 0x05: // rlineto
         if (sp < 2) return STBTT__CSERR("rlineto stack");
         for (; i + 1 < sp; i += 2)
            stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
      // starting from a different place.

      case 0x07: // vlineto
         if (sp < 1) return STBTT__CSERR("vlineto stack");
         goto vlineto;
      case 0x06: // hlineto
         if (sp < 1) return STBTT__CSERR("hlineto stack");
         for (;;) {
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, s[i], 0);
            i++;
      vlineto:
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, 0, s[i]);
            i++;
         }
         break;

      case 0x1F: // hvcurveto
         if (sp < 4) return STBTT__CSERR("hvcurveto stack");
         goto hvcurveto;
      case 0x1E: // vhcurveto
         if (sp < 4) return STBTT__CSERR("vhcurveto stack");
         for (;;) {
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, 0, s[i], s[i+1], s[i+2], s[i+3], (sp - i == 5) ? s[i + 4] : 0.0f);
            i += 4;
      hvcurveto:
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, s[i], 0, s[i+1], s[i+2], (sp - i == 5) ? s[i+4] : 0.0f, s[i+3]);
            i += 4;
         }
         break;

      case 0x08: // rrcurveto
         if (sp < 6) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp; i += 6)
            stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x18: // rcurveline
         if (sp < 8) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp - 2; i += 6)
            stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         if (i + 1 >= sp) return STBTT__CSERR("rcurveline stack");
         stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      case 0x19: // rlinecurve
         if (sp < 8) return STBTT__CSERR("rlinecurve stack");
         for (; i + 1 < sp - 6; i += 2)
            stbtt__csctx_rline_to(c, s[i], s[i+1]);
         if (i + 5 >= sp) return STBTT__CSERR("rlinecurve stack");
         stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x1A: // vvcurveto
      case 0x1B: // hhcurveto
         if (sp < 4) return STBTT__CSERR("(vv|hh)curveto stack");
         f = 0.0;
         if (sp & 1) { f = s[i]; i++; }
         for (; i + 3 < sp; i += 4) {
            if (b0 == 0x1B)
               stbtt__csctx_rccurve_to(c, s[i], f, s[i+1], s[i+2], s[i+3], 0.0);
            else
               stbtt__csctx_rccurve_to(c, f, s[i], s[i+1], s[i+2], 0.0, s[i+3]);
            f = 0.0;
         }
         break;

      case 0x0A: // callsubr
         if (!has_subrs) {
            if (info->fdselect.size)
               subrs = stbtt__cid_get_glyph_subrs(info, glyph_index);
            has_subrs = 1;
         }
         // fallthrough
      case 0x1D: // callgsubr
         if (sp < 1) return STBTT__CSERR("call(g|)subr stack");
         v = (int) s[--sp];
         if (subr_stack_height >= 10) return STBTT__CSERR("recursion limit");
         subr_stack[subr_stack_height++] = b;
         b = stbtt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
         if (b.size == 0) return STBTT__CSERR("subr not found");
         b.cursor = 0;
         clear_stack = 0;
         break;

      case 0x0B: // return
         if (subr_stack_height <= 0) return STBTT__CSERR("return outside subr");
         b = subr_stack[--subr_stack_height];
         clear_stack = 0;
         break;

      case 0x0E: // endchar
         stbtt__csctx_close_shape(c);
         return 1;

      case 0x0C: { // two-byte escape
         float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
         float dx, dy;
         int b1 = stbtt__buf_get8(&b);
         switch (b1) {
         // @TODO These "flex" implementations ignore the flex-depth and resolution,
         // and always draw beziers.
         case 0x22: // hflex
            if (sp < 7) return STBTT__CSERR("hflex stack");
            dx1 = s[0];
            dx2 = s[1];
            dy2 = s[2];
            dx3 = s[3];
            dx4 = s[4];
            dx5 = s[5];
            dx6 = s[6];
            stbtt__csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
            stbtt__csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
            break;

         case 0x23: // flex
            if (sp < 13) return STBTT__CSERR("flex stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = s[10];
            dy6 = s[11];
            //fd is s[12]
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         case 0x24: // hflex1
            if (sp < 9) return STBTT__CSERR("hflex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dx4 = s[5];
            dx5 = s[6];
            dy5 = s[7];
            dx6 = s[8];
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
            stbtt__csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1+dy2+dy5));
            break;

         case 0x25: // flex1
            if (sp < 11) return STBTT__CSERR("flex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = dy6 = s[10];
            dx = dx1+dx2+dx3+dx4+dx5;
            dy = dy1+dy2+dy3+dy4+dy5;
            if (STBTT_fabs(dx) > STBTT_fabs(dy))
               dy6 = -dy;
            else
               dx6 = -dx;
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         default:
            return STBTT__CSERR("unimplemented");
         }
      } break;

      default:
         if (b0 != 255 && b0 != 28 && (b0 < 32 || b0 > 254))
            return STBTT__CSERR("reserved operator");

         // push immediate
         if (b0 == 255) {
            f = (float)(stbtt_int32)stbtt__buf_get32(&b) / 0x10000;
         } else {
            stbtt__buf_skip(&b, -1);
            f = (float)(stbtt_int16)stbtt__cff_int(&b);
         }
         if (sp >= 48) return STBTT__CSERR("push stack overflow");
         s[sp++] = f;
         clear_stack = 0;
         break;
      }
      if (clear_stack) sp = 0;
   }
   return STBTT__CSERR("no endchar");

#undef STBTT__CSERR
}

static int stbtt__GetGlyphShapeT2(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   // runs the charstring twice, once to count and once to output (to avoid realloc)
   stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1);
   stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);
   if (stbtt__run_charstring(info, glyph_index, &count_ctx)) {
      *pvertices = (stbtt_vertex*)STBTT_malloc(count_ctx.num_vertices*sizeof(stbtt_vertex), info->userdata);
      output_ctx.pvertices = *pvertices;
      if (stbtt__run_charstring(info, glyph_index, &output_ctx)) {
         STBTT_assert(output_ctx.num_vertices == count_ctx.num_vertices);
         return output_ctx.num_vertices;
      }
   }
   *pvertices = NULL;
   return 0;
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   stbtt__csctx c = STBTT__CSCTX_INIT(1);
   int r = stbtt__run_charstring(info, glyph_index, &c);
   if (x0)  *x0 = r ? c.min_x : 0;
   if (y0)  *y0 = r ? c.min_y : 0;
   if (x1)  *x1 = r ? c.max_x : 0;
   if (y1)  *y1 = r ? c.max_y : 0;
   return r ? c.num_vertices : 0;
}

STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   if (!info->cff.size)
      return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
   else
      return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

STBTT_DEF void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing)
{
   stbtt_uint16 numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
   if (glyph_index < numOfLongHorMetrics) {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
   } else {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
   }
}

STBTT_DEF int  stbtt_GetKerningTableLength(const stbtt_fontinfo *info)
{
   stbtt_uint8 *data = info->data + info->kern;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   return ttUSHORT(data+10);
}

STBTT_DEF int stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry* table, int table_length)
{
   stbtt_uint8 *data = info->data + info->kern;
   int k, length;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   length = ttUSHORT(data+10);
   if (table_length < length)
      length = table_length;

   for (k = 0; k < length; k++)
   {
      table[k].glyph1 = ttUSHORT(data+18+(k*6));
      table[k].glyph2 = ttUSHORT(data+20+(k*6));
      table[k].advance = ttSHORT(data+22+(k*6));
   }

   return length;
}

static int  stbtt__GetGlyphKernInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2)
{
   stbtt_uint8 *data = info->data + info->kern;
   stbtt_uint32 needle, straw;
   int l, r, m;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   l = 0;
   r = ttUSHORT(data+10) - 1;
   needle = glyph1 << 16 | glyph2;
   while (l <= r) {
      m = (l + r) >> 1;
      straw = ttULONG(data+18+(m*6)); // note: unaligned read
      if (needle < straw)
         r = m - 1;
      else if (needle > straw)
         l = m + 1;
      else
         return ttSHORT(data+22+(m*6));
   }
   return 0;
}

static stbtt_int32  stbtt__GetCoverageIndex(stbtt_uint8 *coverageTable, int glyph)
{
    stbtt_uint16 coverageFormat = ttUSHORT(coverageTable);
    switch(coverageFormat) {
        case 1: {
            stbtt_uint16 glyphCount = ttUSHORT(coverageTable + 2);

            // Binary search.
            stbtt_int32 l=0, r=glyphCount-1, m;
            int straw, needle=glyph;
            while (l <= r) {
                stbtt_uint8 *glyphArray = coverageTable + 4;
                stbtt_uint16 glyphID;
                m = (l + r) >> 1;
                glyphID = ttUSHORT(glyphArray + 2 * m);
                straw = glyphID;
                if (needle < straw)
                    r = m - 1;
                else if (needle > straw)
                    l = m + 1;
                else {
                     return m;
                }
            }
        } break;

        case 2: {
            stbtt_uint16 rangeCount = ttUSHORT(coverageTable + 2);
            stbtt_uint8 *rangeArray = coverageTable + 4;

            // Binary search.
            stbtt_int32 l=0, r=rangeCount-1, m;
            int strawStart, strawEnd, needle=glyph;
            while (l <= r) {
                stbtt_uint8 *rangeRecord;
                m = (l + r) >> 1;
                rangeRecord = rangeArray + 6 * m;
                strawStart = ttUSHORT(rangeRecord);
                strawEnd = ttUSHORT(rangeRecord + 2);
                if (needle < strawStart)
                    r = m - 1;
                else if (needle > strawEnd)
                    l = m + 1;
                else {
                    stbtt_uint16 startCoverageIndex = ttUSHORT(rangeRecord + 4);
                    return startCoverageIndex + glyph - strawStart;
                }
            }
        } break;

        default: {
            // There are no other cases.
            STBTT_assert(0);
        } break;
    }

    return -1;
}

static stbtt_int32  stbtt__GetGlyphClass(stbtt_uint8 *classDefTable, int glyph)
{
    stbtt_uint16 classDefFormat = ttUSHORT(classDefTable);
    switch(classDefFormat)
    {
        case 1: {
            stbtt_uint16 startGlyphID = ttUSHORT(classDefTable + 2);
            stbtt_uint16 glyphCount = ttUSHORT(classDefTable + 4);
            stbtt_uint8 *classDef1ValueArray = classDefTable + 6;

            if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
                return (stbtt_int32)ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));

            classDefTable = classDef1ValueArray + 2 * glyphCount;
        } break;

        case 2: {
            stbtt_uint16 classRangeCount = ttUSHORT(classDefTable + 2);
            stbtt_uint8 *classRangeRecords = classDefTable + 4;

            // Binary search.
            stbtt_int32 l=0, r=classRangeCount-1, m;
            int strawStart, strawEnd, needle=glyph;
            while (l <= r) {
                stbtt_uint8 *classRangeRecord;
                m = (l + r) >> 1;
                classRangeRecord = classRangeRecords + 6 * m;
                strawStart = ttUSHORT(classRangeRecord);
                strawEnd = ttUSHORT(classRangeRecord + 2);
                if (needle < strawStart)
                    r = m - 1;
                else if (needle > strawEnd)
                    l = m + 1;
                else
                    return (stbtt_int32)ttUSHORT(classRangeRecord + 4);
            }

            classDefTable = classRangeRecords + 6 * classRangeCount;
        } break;

        default: {
            // There are no other cases.
            STBTT_assert(0);
        } break;
    }

    return -1;
}

// Define to STBTT_assert(x) if you want to break on unimplemented formats.
#define STBTT_GPOS_TODO_assert(x)

static stbtt_int32  stbtt__GetGlyphGPOSInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2)
{
    stbtt_uint16 lookupListOffset;
    stbtt_uint8 *lookupList;
    stbtt_uint16 lookupCount;
    stbtt_uint8 *data;
    stbtt_int32 i;

    if (!info->gpos) return 0;

    data = info->data + info->gpos;

    if (ttUSHORT(data+0) != 1) return 0; // Major version 1
    if (ttUSHORT(data+2) != 0) return 0; // Minor version 0

    lookupListOffset = ttUSHORT(data+8);
    lookupList = data + lookupListOffset;
    lookupCount = ttUSHORT(lookupList);

    for (i=0; i<lookupCount; ++i) {
        stbtt_uint16 lookupOffset = ttUSHORT(lookupList + 2 + 2 * i);
        stbtt_uint8 *lookupTable = lookupList + lookupOffset;

        stbtt_uint16 lookupType = ttUSHORT(lookupTable);
        stbtt_uint16 subTableCount = ttUSHORT(lookupTable + 4);
        stbtt_uint8 *subTableOffsets = lookupTable + 6;
        switch(lookupType) {
            case 2: { // Pair Adjustment Positioning Subtable
                stbtt_int32 sti;
                for (sti=0; sti<subTableCount; sti++) {
                    stbtt_uint16 subtableOffset = ttUSHORT(subTableOffsets + 2 * sti);
                    stbtt_uint8 *table = lookupTable + subtableOffset;
                    stbtt_uint16 posFormat = ttUSHORT(table);
                    stbtt_uint16 coverageOffset = ttUSHORT(table + 2);
                    stbtt_int32 coverageIndex = stbtt__GetCoverageIndex(table + coverageOffset, glyph1);
                    if (coverageIndex == -1) continue;

                    switch (posFormat) {
                        case 1: {
                            stbtt_int32 l, r, m;
                            int straw, needle;
                            stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
                            stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);
                            stbtt_int32 valueRecordPairSizeInBytes = 2;
                            stbtt_uint16 pairSetCount = ttUSHORT(table + 8);
                            stbtt_uint16 pairPosOffset = ttUSHORT(table + 10 + 2 * coverageIndex);
                            stbtt_uint8 *pairValueTable = table + pairPosOffset;
                            stbtt_uint16 pairValueCount = ttUSHORT(pairValueTable);
                            stbtt_uint8 *pairValueArray = pairValueTable + 2;
                            // TODO: Support more formats.
                            STBTT_GPOS_TODO_assert(valueFormat1 == 4);
                            if (valueFormat1 != 4) return 0;
                            STBTT_GPOS_TODO_assert(valueFormat2 == 0);
                            if (valueFormat2 != 0) return 0;

                            STBTT_assert(coverageIndex < pairSetCount);
                            STBTT__NOTUSED(pairSetCount);

                            needle=glyph2;
                            r=pairValueCount-1;
                            l=0;

                            // Binary search.
                            while (l <= r) {
                                stbtt_uint16 secondGlyph;
                                stbtt_uint8 *pairValue;
                                m = (l + r) >> 1;
                                pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
                                secondGlyph = ttUSHORT(pairValue);
                                straw = secondGlyph;
                                if (needle < straw)
                                    r = m - 1;
                                else if (needle > straw)
                                    l = m + 1;
                                else {
                                    stbtt_int16 xAdvance = ttSHORT(pairValue + 2);
                                    return xAdvance;
                                }
                            }
                        } break;

                        case 2: {
                            stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
                            stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);

                            stbtt_uint16 classDef1Offset = ttUSHORT(table + 8);
                            stbtt_uint16 classDef2Offset = ttUSHORT(table + 10);
                            int glyph1class = stbtt__GetGlyphClass(table + classDef1Offset, glyph1);
                            int glyph2class = stbtt__GetGlyphClass(table + classDef2Offset, glyph2);

                            stbtt_uint16 class1Count = ttUSHORT(table + 12);
                            stbtt_uint16 class2Count = ttUSHORT(table + 14);
                            STBTT_assert(glyph1class < class1Count);
                            STBTT_assert(glyph2class < class2Count);

                            // TODO: Support more formats.
                            STBTT_GPOS_TODO_assert(valueFormat1 == 4);
                            if (valueFormat1 != 4) return 0;
                            STBTT_GPOS_TODO_assert(valueFormat2 == 0);
                            if (valueFormat2 != 0) return 0;

                            if (glyph1class >= 0 && glyph1class < class1Count && glyph2class >= 0 && glyph2class < class2Count) {
                                stbtt_uint8 *class1Records = table + 16;
                                stbtt_uint8 *class2Records = class1Records + 2 * (glyph1class * class2Count);
                                stbtt_int16 xAdvance = ttSHORT(class2Records + 2 * glyph2class);
                                return xAdvance;
                            }
                        } break;

                        default: {
                            // There are no other cases.
                            STBTT_assert(0);
                            break;
                        };
                    }
                }
                break;
            };

            default:
                // TODO: Implement other stuff.
                break;
        }
    }

    return 0;
}

STBTT_DEF int  stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int g1, int g2)
{
   int xAdvance = 0;

   if (info->gpos)
      xAdvance += stbtt__GetGlyphGPOSInfoAdvance(info, g1, g2);
   else if (info->kern)
      xAdvance += stbtt__GetGlyphKernInfoAdvance(info, g1, g2);

   return xAdvance;
}

STBTT_DEF int  stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2)
{
   if (!info->kern && !info->gpos) // if no kerning table, don't waste time looking up both codepoint->glyphs
      return 0;
   return stbtt_GetGlyphKernAdvance(info, stbtt_FindGlyphIndex(info,ch1), stbtt_FindGlyphIndex(info,ch2));
}

STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing)
{
   stbtt_GetGlyphHMetrics(info, stbtt_FindGlyphIndex(info,codepoint), advanceWidth, leftSideBearing);
}

STBTT_DEF void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap)
{
   if (ascent ) *ascent  = ttSHORT(info->data+info->hhea + 4);
   if (descent) *descent = ttSHORT(info->data+info->hhea + 6);
   if (lineGap) *lineGap = ttSHORT(info->data+info->hhea + 8);
}

STBTT_DEF int  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap)
{
   int tab = stbtt__find_table(info->data, info->fontstart, "OS/2");
   if (!tab)
      return 0;
   if (typoAscent ) *typoAscent  = ttSHORT(info->data+tab + 68);
   if (typoDescent) *typoDescent = ttSHORT(info->data+tab + 70);
   if (typoLineGap) *typoLineGap = ttSHORT(info->data+tab + 72);
   return 1;
}

STBTT_DEF void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1)
{
   *x0 = ttSHORT(info->data + info->head + 36);
   *y0 = ttSHORT(info->data + info->head + 38);
   *x1 = ttSHORT(info->data + info->head + 40);
   *y1 = ttSHORT(info->data + info->head + 42);
}

STBTT_DEF float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float height)
{
   int fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6);
   return (float) height / fheight;
}

STBTT_DEF float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels)
{
   int unitsPerEm = ttUSHORT(info->data + info->head + 18);
   return pixels / unitsPerEm;
}

STBTT_DEF void stbtt_FreeShape(const stbtt_fontinfo *info, stbtt_vertex *v)
{
   STBTT_free(v, info->userdata);
}

STBTT_DEF stbtt_uint8 *stbtt_FindSVGDoc(const stbtt_fontinfo *info, int gl)
{
   int i;
   stbtt_uint8 *data = info->data;
   stbtt_uint8 *svg_doc_list = data + stbtt__get_svg((stbtt_fontinfo *) info);

   int numEntries = ttUSHORT(svg_doc_list);
   stbtt_uint8 *svg_docs = svg_doc_list + 2;

   for(i=0; i<numEntries; i++) {
      stbtt_uint8 *svg_doc = svg_docs + (12 * i);
      if ((gl >= ttUSHORT(svg_doc)) && (gl <= ttUSHORT(svg_doc + 2)))
         return svg_doc;
   }
   return 0;
}

STBTT_DEF int stbtt_GetGlyphSVG(const stbtt_fontinfo *info, int gl, const char **svg)
{
   stbtt_uint8 *data = info->data;
   stbtt_uint8 *svg_doc;

   if (info->svg == 0)
      return 0;

   svg_doc = stbtt_FindSVGDoc(info, gl);
   if (svg_doc != NULL) {
      *svg = (char *) data + info->svg + ttULONG(svg_doc + 4);
      return ttULONG(svg_doc + 8);
   } else {
      return 0;
   }
}

STBTT_DEF int stbtt_GetCodepointSVG(const stbtt_fontinfo *info, int unicode_codepoint, const char **svg)
{
   return stbtt_GetGlyphSVG(info, stbtt_FindGlyphIndex(info, unicode_codepoint), svg);
}

//////////////////////////////////////////////////////////////////////////////
//
// antialiasing software rasterizer
//

STBTT_DEF void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   int x0=0,y0=0,x1,y1; // =0 suppresses compiler warning
   if (!stbtt_GetGlyphBox(font, glyph, &x0,&y0,&x1,&y1)) {
      // e.g. space character
      if (ix0) *ix0 = 0;
      if (iy0) *iy0 = 0;
      if (ix1) *ix1 = 0;
      if (iy1) *iy1 = 0;
   } else {
      // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
      if (ix0) *ix0 = STBTT_ifloor( x0 * scale_x + shift_x);
      if (iy0) *iy0 = STBTT_ifloor(-y1 * scale_y + shift_y);
      if (ix1) *ix1 = STBTT_iceil ( x1 * scale_x + shift_x);
      if (iy1) *iy1 = STBTT_iceil (-y0 * scale_y + shift_y);
   }
}

STBTT_DEF void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y,0.0f,0.0f, ix0, iy0, ix1, iy1);
}

STBTT_DEF void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, stbtt_FindGlyphIndex(font,codepoint), scale_x, scale_y,shift_x,shift_y, ix0,iy0,ix1,iy1);
}

STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale_x, scale_y,0.0f,0.0f, ix0,iy0,ix1,iy1);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rasterizer

typedef struct stbtt__hheap_chunk
{
   struct stbtt__hheap_chunk *next;
} stbtt__hheap_chunk;

typedef struct stbtt__hheap
{
   struct stbtt__hheap_chunk *head;
   void   *first_free;
   int    num_remaining_in_head_chunk;
} stbtt__hheap;

static void *stbtt__hheap_alloc(stbtt__hheap *hh, size_t size, void *userdata)
{
   if (hh->first_free) {
      void *p = hh->first_free;
      hh->first_free = * (void **) p;
      return p;
   } else {
      if (hh->num_remaining_in_head_chunk == 0) {
         int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
         stbtt__hheap_chunk *c = (stbtt__hheap_chunk *) STBTT_malloc(sizeof(stbtt__hheap_chunk) + size * count, userdata);
         if (c == NULL)
            return NULL;
         c->next = hh->head;
         hh->head = c;
         hh->num_remaining_in_head_chunk = count;
      }
      --hh->num_remaining_in_head_chunk;
      return (char *) (hh->head) + sizeof(stbtt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
   }
}

static void stbtt__hheap_free(stbtt__hheap *hh, void *p)
{
   *(void **) p = hh->first_free;
   hh->first_free = p;
}

static void stbtt__hheap_cleanup(stbtt__hheap *hh, void *userdata)
{
   stbtt__hheap_chunk *c = hh->head;
   while (c) {
      stbtt__hheap_chunk *n = c->next;
      STBTT_free(c, userdata);
      c = n;
   }
}

typedef struct stbtt__edge {
   float x0,y0, x1,y1;
   int invert;
} stbtt__edge;


typedef struct stbtt__active_edge
{
   struct stbtt__active_edge *next;
   #if STBTT_RASTERIZER_VERSION==1
   int x,dx;
   float ey;
   int direction;
   #elif STBTT_RASTERIZER_VERSION==2
   float fx,fdx,fdy;
   float direction;
   float sy;
   float ey;
   #else
   #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
   #endif
} stbtt__active_edge;

#if STBTT_RASTERIZER_VERSION == 1
#define STBTT_FIXSHIFT   10
#define STBTT_FIX        (1 << STBTT_FIXSHIFT)
#define STBTT_FIXMASK    (STBTT_FIX-1)

static stbtt__active_edge *stbtt__new_active(stbtt__hheap *hh, stbtt__edge *e, int off_x, float start_point, void *userdata)
{
   stbtt__active_edge *z = (stbtt__active_edge *) stbtt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   STBTT_assert(z != NULL);
   if (!z) return z;

   // round dx down to avoid overshooting
   if (dxdy < 0)
      z->dx = -STBTT_ifloor(STBTT_FIX * -dxdy);
   else
      z->dx = STBTT_ifloor(STBTT_FIX * dxdy);

   z->x = STBTT_ifloor(STBTT_FIX * e->x0 + z->dx * (start_point - e->y0)); // use z->dx so when we offset later it's by the same amount
   z->x -= off_x * STBTT_FIX;

   z->ey = e->y1;
   z->next = 0;
   z->direction = e->invert ? 1 : -1;
   return z;
}
#elif STBTT_RASTERIZER_VERSION == 2
static stbtt__active_edge *stbtt__new_active(stbtt__hheap *hh, stbtt__edge *e, int off_x, float start_point, void *userdata)
{
   stbtt__active_edge *z = (stbtt__active_edge *) stbtt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   STBTT_assert(z != NULL);
   //STBTT_assert(e->y0 <= start_point);
   if (!z) return z;
   z->fdx = dxdy;
   z->fdy = dxdy != 0.0f ? (1.0f/dxdy) : 0.0f;
   z->fx = e->x0 + dxdy * (start_point - e->y0);
   z->fx -= off_x;
   z->direction = e->invert ? 1.0f : -1.0f;
   z->sy = e->y0;
   z->ey = e->y1;
   z->next = 0;
   return z;
}
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

#if STBTT_RASTERIZER_VERSION == 1
// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void stbtt__fill_active_edges(unsigned char *scanline, int len, stbtt__active_edge *e, int max_weight)
{
   // non-zero winding fill
   int x0=0, w=0;

   while (e) {
      if (w == 0) {
         // if we're currently at zero, we need to record the edge start point
         x0 = e->x; w += e->direction;
      } else {
         int x1 = e->x; w += e->direction;
         // if we went to zero, we need to draw
         if (w == 0) {
            int i = x0 >> STBTT_FIXSHIFT;
            int j = x1 >> STBTT_FIXSHIFT;

            if (i < len && j >= 0) {
               if (i == j) {
                  // x0,x1 are the same pixel, so compute combined coverage
                  scanline[i] = scanline[i] + (stbtt_uint8) ((x1 - x0) * max_weight >> STBTT_FIXSHIFT);
               } else {
                  if (i >= 0) // add antialiasing for x0
                     scanline[i] = scanline[i] + (stbtt_uint8) (((STBTT_FIX - (x0 & STBTT_FIXMASK)) * max_weight) >> STBTT_FIXSHIFT);
                  else
                     i = -1; // clip

                  if (j < len) // add antialiasing for x1
                     scanline[j] = scanline[j] + (stbtt_uint8) (((x1 & STBTT_FIXMASK) * max_weight) >> STBTT_FIXSHIFT);
                  else
                     j = len; // clip

                  for (++i; i < j; ++i) // fill pixels between x0 and x1
                     scanline[i] = scanline[i] + (stbtt_uint8) max_weight;
               }
            }
         }
      }

      e = e->next;
   }
}

static void stbtt__rasterize_sorted_edges(stbtt__bitmap *result, stbtt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   stbtt__hheap hh = { 0, 0, 0 };
   stbtt__active_edge *active = NULL;
   int y,j=0;
   int max_weight = (255 / vsubsample);  // weight per vertical scanline
   int s; // vertical subsample index
   unsigned char scanline_data[512], *scanline;

   if (result->w > 512)
      scanline = (unsigned char *) STBTT_malloc(result->w, userdata);
   else
      scanline = scanline_data;

   y = off_y * vsubsample;
   e[n].y0 = (off_y + result->h) * (float) vsubsample + 1;

   while (j < result->h) {
      STBTT_memset(scanline, 0, result->w);
      for (s=0; s < vsubsample; ++s) {
         // find center of pixel for this scanline
         float scan_y = y + 0.5f;
         stbtt__active_edge **step = &active;

         // update all active edges;
         // remove all active edges that terminate before the center of this scanline
         while (*step) {
            stbtt__active_edge * z = *step;
            if (z->ey <= scan_y) {
               *step = z->next; // delete from list
               STBTT_assert(z->direction);
               z->direction = 0;
               stbtt__hheap_free(&hh, z);
            } else {
               z->x += z->dx; // advance to position for current scanline
               step = &((*step)->next); // advance through list
            }
         }

         // resort the list if needed
         for(;;) {
            int changed=0;
            step = &active;
            while (*step && (*step)->next) {
               if ((*step)->x > (*step)->next->x) {
                  stbtt__active_edge *t = *step;
                  stbtt__active_edge *q = t->next;

                  t->next = q->next;
                  q->next = t;
                  *step = q;
                  changed = 1;
               }
               step = &(*step)->next;
            }
            if (!changed) break;
         }

         // insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
         while (e->y0 <= scan_y) {
            if (e->y1 > scan_y) {
               stbtt__active_edge *z = stbtt__new_active(&hh, e, off_x, scan_y, userdata);
               if (z != NULL) {
                  // find insertion point
                  if (active == NULL)
                     active = z;
                  else if (z->x < active->x) {
                     // insert at front
                     z->next = active;
                     active = z;
                  } else {
                     // find thing to insert AFTER
                     stbtt__active_edge *p = active;
                     while (p->next && p->next->x < z->x)
                        p = p->next;
                     // at this point, p->next->x is NOT < z->x
                     z->next = p->next;
                     p->next = z;
                  }
               }
            }
            ++e;
         }

         // now process all active edges in XOR fashion
         if (active)
            stbtt__fill_active_edges(scanline, result->w, active, max_weight);

         ++y;
      }
      STBTT_memcpy(result->pixels + j * result->stride, scanline, result->w);
      ++j;
   }

   stbtt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      STBTT_free(scanline, userdata);
}

#elif STBTT_RASTERIZER_VERSION == 2

// the edge passed in here does not cross the vertical line at x or the vertical line at x+1
// (i.e. it has already been clipped to those)
static void stbtt__handle_clipped_edge(float *scanline, int x, stbtt__active_edge *e, float x0, float y0, float x1, float y1)
{
   if (y0 == y1) return;
   STBTT_assert(y0 < y1);
   STBTT_assert(e->sy <= e->ey);
   if (y0 > e->ey) return;
   if (y1 < e->sy) return;
   if (y0 < e->sy) {
      x0 += (x1-x0) * (e->sy - y0) / (y1-y0);
      y0 = e->sy;
   }
   if (y1 > e->ey) {
      x1 += (x1-x0) * (e->ey - y1) / (y1-y0);
      y1 = e->ey;
   }

   if (x0 == x)
      STBTT_assert(x1 <= x+1);
   else if (x0 == x+1)
      STBTT_assert(x1 >= x);
   else if (x0 <= x)
      STBTT_assert(x1 <= x);
   else if (x0 >= x+1)
      STBTT_assert(x1 >= x+1);
   else
      STBTT_assert(x1 >= x && x1 <= x+1);

   if (x0 <= x && x1 <= x)
      scanline[x] += e->direction * (y1-y0);
   else if (x0 >= x+1 && x1 >= x+1)
      ;
   else {
      STBTT_assert(x0 >= x && x0 <= x+1 && x1 >= x && x1 <= x+1);
      scanline[x] += e->direction * (y1-y0) * (1-((x0-x)+(x1-x))/2); // coverage = 1 - average x position
   }
}

static void stbtt__fill_active_edges_new(float *scanline, float *scanline_fill, int len, stbtt__active_edge *e, float y_top)
{
   float y_bottom = y_top+1;

   while (e) {
      // brute force every pixel

      // compute intersection points with top & bottom
      STBTT_assert(e->ey >= y_top);

      if (e->fdx == 0) {
         float x0 = e->fx;
         if (x0 < len) {
            if (x0 >= 0) {
               stbtt__handle_clipped_edge(scanline,(int) x0,e, x0,y_top, x0,y_bottom);
               stbtt__handle_clipped_edge(scanline_fill-1,(int) x0+1,e, x0,y_top, x0,y_bottom);
            } else {
               stbtt__handle_clipped_edge(scanline_fill-1,0,e, x0,y_top, x0,y_bottom);
            }
         }
      } else {
         float x0 = e->fx;
         float dx = e->fdx;
         float xb = x0 + dx;
         float x_top, x_bottom;
         float sy0,sy1;
         float dy = e->fdy;
         STBTT_assert(e->sy <= y_bottom && e->ey >= y_top);

         // compute endpoints of line segment clipped to this scanline (if the
         // line segment starts on this scanline. x0 is the intersection of the
         // line with y_top, but that may be off the line segment.
         if (e->sy > y_top) {
            x_top = x0 + dx * (e->sy - y_top);
            sy0 = e->sy;
         } else {
            x_top = x0;
            sy0 = y_top;
         }
         if (e->ey < y_bottom) {
            x_bottom = x0 + dx * (e->ey - y_top);
            sy1 = e->ey;
         } else {
            x_bottom = xb;
            sy1 = y_bottom;
         }

         if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
            // from here on, we don't have to range check x values

            if ((int) x_top == (int) x_bottom) {
               float height;
               // simple case, only spans one pixel
               int x = (int) x_top;
               height = sy1 - sy0;
               STBTT_assert(x >= 0 && x < len);
               scanline[x] += e->direction * (1-((x_top - x) + (x_bottom-x))/2)  * height;
               scanline_fill[x] += e->direction * height; // everything right of this pixel is filled
            } else {
               int x,x1,x2;
               float y_crossing, step, sign, area;
               // covers 2+ pixels
               if (x_top > x_bottom) {
                  // flip scanline vertically; signed area is the same
                  float t;
                  sy0 = y_bottom - (sy0 - y_top);
                  sy1 = y_bottom - (sy1 - y_top);
                  t = sy0, sy0 = sy1, sy1 = t;
                  t = x_bottom, x_bottom = x_top, x_top = t;
                  dx = -dx;
                  dy = -dy;
                  t = x0, x0 = xb, xb = t;
               }

               x1 = (int) x_top;
               x2 = (int) x_bottom;
               // compute intersection with y axis at x1+1
               y_crossing = (x1+1 - x0) * dy + y_top;

               sign = e->direction;
               // area of the rectangle covered from y0..y_crossing
               area = sign * (y_crossing-sy0);
               // area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
               scanline[x1] += area * (1-((x_top - x1)+(x1+1-x1))/2);

               step = sign * dy;
               for (x = x1+1; x < x2; ++x) {
                  scanline[x] += area + step/2;
                  area += step;
               }
               y_crossing += dy * (x2 - (x1+1));

               STBTT_assert(STBTT_fabs(area) <= 1.01f);

               scanline[x2] += area + sign * (1-((x2-x2)+(x_bottom-x2))/2) * (sy1-y_crossing);

               scanline_fill[x2] += sign * (sy1-sy0);
            }
         } else {
            // if edge goes outside of box we're drawing, we require
            // clipping logic. since this does not match the intended use
            // of this library, we use a different, very slow brute
            // force implementation
            int x;
            for (x=0; x < len; ++x) {
               // cases:
               //
               // there can be up to two intersections with the pixel. any intersection
               // with left or right edges can be handled by splitting into two (or three)
               // regions. intersections with top & bottom do not necessitate case-wise logic.
               //
               // the old way of doing this found the intersections with the left & right edges,
               // then used some simple logic to produce up to three segments in sorted order
               // from top-to-bottom. however, this had a problem: if an x edge was epsilon
               // across the x border, then the corresponding y position might not be distinct
               // from the other y segment, and it might ignored as an empty segment. to avoid
               // that, we need to explicitly produce segments based on x positions.

               // rename variables to clearly-defined pairs
               float y0 = y_top;
               float x1 = (float) (x);
               float x2 = (float) (x+1);
               float x3 = xb;
               float y3 = y_bottom;

               // x = e->x + e->dx * (y-y_top)
               // (y-y_top) = (x - e->x) / e->dx
               // y = (x - e->x) / e->dx + y_top
               float y1 = (x - x0) / dx + y_top;
               float y2 = (x+1 - x0) / dx + y_top;

               if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else {  // one segment
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x3,y3);
               }
            }
         }
      }
      e = e->next;
   }
}

// directly AA rasterize edges w/o supersampling
static void stbtt__rasterize_sorted_edges(stbtt__bitmap *result, stbtt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   stbtt__hheap hh = { 0, 0, 0 };
   stbtt__active_edge *active = NULL;
   int y,j=0, i;
   float scanline_data[129], *scanline, *scanline2;

   STBTT__NOTUSED(vsubsample);

   if (result->w > 64)
      scanline = (float *) STBTT_malloc((result->w*2+1) * sizeof(float), userdata);
   else
      scanline = scanline_data;

   scanline2 = scanline + result->w;

   y = off_y;
   e[n].y0 = (float) (off_y + result->h) + 1;

   while (j < result->h) {
      // find center of pixel for this scanline
      float scan_y_top    = y + 0.0f;
      float scan_y_bottom = y + 1.0f;
      stbtt__active_edge **step = &active;

      STBTT_memset(scanline , 0, result->w*sizeof(scanline[0]));
      STBTT_memset(scanline2, 0, (result->w+1)*sizeof(scanline[0]));

      // update all active edges;
      // remove all active edges that terminate before the top of this scanline
      while (*step) {
         stbtt__active_edge * z = *step;
         if (z->ey <= scan_y_top) {
            *step = z->next; // delete from list
            STBTT_assert(z->direction);
            z->direction = 0;
            stbtt__hheap_free(&hh, z);
         } else {
            step = &((*step)->next); // advance through list
         }
      }

      // insert all edges that start before the bottom of this scanline
      while (e->y0 <= scan_y_bottom) {
         if (e->y0 != e->y1) {
            stbtt__active_edge *z = stbtt__new_active(&hh, e, off_x, scan_y_top, userdata);
            if (z != NULL) {
               if (j == 0 && off_y != 0) {
                  if (z->ey < scan_y_top) {
                     // this can happen due to subpixel positioning and some kind of fp rounding error i think
                     z->ey = scan_y_top;
                  }
               }
               STBTT_assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
               // insert at front
               z->next = active;
               active = z;
            }
         }
         ++e;
      }

      // now process all active edges
      if (active)
         stbtt__fill_active_edges_new(scanline, scanline2+1, result->w, active, scan_y_top);

      {
         float sum = 0;
         for (i=0; i < result->w; ++i) {
            float k;
            int m;
            sum += scanline2[i];
            k = scanline[i] + sum;
            k = (float) STBTT_fabs(k)*255 + 0.5f;
            m = (int) k;
            if (m > 255) m = 255;
            result->pixels[j*result->stride + i] = (unsigned char) m;
         }
      }
      // advance all the edges
      step = &active;
      while (*step) {
         stbtt__active_edge *z = *step;
         z->fx += z->fdx; // advance to position for current scanline
         step = &((*step)->next); // advance through list
      }

      ++y;
      ++j;
   }

   stbtt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      STBTT_free(scanline, userdata);
}
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)

static void stbtt__sort_edges_ins_sort(stbtt__edge *p, int n)
{
   int i,j;
   for (i=1; i < n; ++i) {
      stbtt__edge t = p[i], *a = &t;
      j = i;
      while (j > 0) {
         stbtt__edge *b = &p[j-1];
         int c = STBTT__COMPARE(a,b);
         if (!c) break;
         p[j] = p[j-1];
         --j;
      }
      if (i != j)
         p[j] = t;
   }
}

static void stbtt__sort_edges_quicksort(stbtt__edge *p, int n)
{
   /* threshold for transitioning to insertion sort */
   while (n > 12) {
      stbtt__edge t;
      int c01,c12,c,m,i,j;

      /* compute median of three */
      m = n >> 1;
      c01 = STBTT__COMPARE(&p[0],&p[m]);
      c12 = STBTT__COMPARE(&p[m],&p[n-1]);
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
      if (c01 != c12) {
         /* otherwise, we'll need to swap something else to middle */
         int z;
         c = STBTT__COMPARE(&p[0],&p[n-1]);
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */
         z = (c == c12) ? 0 : n-1;
         t = p[z];
         p[z] = p[m];
         p[m] = t;
      }
      /* now p[m] is the median-of-three */
      /* swap it to the beginning so it won't move around */
      t = p[0];
      p[0] = p[m];
      p[m] = t;

      /* partition loop */
      i=1;
      j=n-1;
      for(;;) {
         /* handling of equality is crucial here */
         /* for sentinels & efficiency with duplicates */
         for (;;++i) {
            if (!STBTT__COMPARE(&p[i], &p[0])) break;
         }
         for (;;--j) {
            if (!STBTT__COMPARE(&p[0], &p[j])) break;
         }
         /* make sure we haven't crossed */
         if (i >= j) break;
         t = p[i];
         p[i] = p[j];
         p[j] = t;

         ++i;
         --j;
      }
      /* recurse on smaller side, iterate on larger */
      if (j < (n-i)) {
         stbtt__sort_edges_quicksort(p,j);
         p = p+i;
         n = n-i;
      } else {
         stbtt__sort_edges_quicksort(p+i, n-i);
         n = j;
      }
   }
}

static void stbtt__sort_edges(stbtt__edge *p, int n)
{
   stbtt__sort_edges_quicksort(p, n);
   stbtt__sort_edges_ins_sort(p, n);
}

typedef struct
{
   float x,y;
} stbtt__point;

static void stbtt__rasterize(stbtt__bitmap *result, stbtt__point *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void *userdata)
{
   float y_scale_inv = invert ? -scale_y : scale_y;
   stbtt__edge *e;
   int n,i,j,k,m;
#if STBTT_RASTERIZER_VERSION == 1
   int vsubsample = result->h < 8 ? 15 : 5;
#elif STBTT_RASTERIZER_VERSION == 2
   int vsubsample = 1;
#else
   #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif
   // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

   // now we have to blow out the windings into explicit edge lists
   n = 0;
   for (i=0; i < windings; ++i)
      n += wcount[i];

   e = (stbtt__edge *) STBTT_malloc(sizeof(*e) * (n+1), userdata); // add an extra one as a sentinel
   if (e == 0) return;
   n = 0;

   m=0;
   for (i=0; i < windings; ++i) {
      stbtt__point *p = pts + m;
      m += wcount[i];
      j = wcount[i]-1;
      for (k=0; k < wcount[i]; j=k++) {
         int a=k,b=j;
         // skip the edge if horizontal
         if (p[j].y == p[k].y)
            continue;
         // add edge from j to k to the list
         e[n].invert = 0;
         if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
            e[n].invert = 1;
            a=j,b=k;
         }
         e[n].x0 = p[a].x * scale_x + shift_x;
         e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
         e[n].x1 = p[b].x * scale_x + shift_x;
         e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
         ++n;
      }
   }

   // now sort the edges by their highest point (should snap to integer, and then by x)
   //STBTT_sort(e, n, sizeof(e[0]), stbtt__edge_compare);
   stbtt__sort_edges(e, n);

   // now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
   stbtt__rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

   STBTT_free(e, userdata);
}

static void stbtt__add_point(stbtt__point *points, int n, float x, float y)
{
   if (!points) return; // during first pass, it's unallocated
   points[n].x = x;
   points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
static int stbtt__tesselate_curve(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
   // midpoint
   float mx = (x0 + 2*x1 + x2)/4;
   float my = (y0 + 2*y1 + y2)/4;
   // versus directly drawn line
   float dx = (x0+x2)/2 - mx;
   float dy = (y0+y2)/2 - my;
   if (n > 16) // 65536 segments on one curve better be enough!
      return 1;
   if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
      stbtt__tesselate_curve(points, num_points, x0,y0, (x0+x1)/2.0f,(y0+y1)/2.0f, mx,my, objspace_flatness_squared,n+1);
      stbtt__tesselate_curve(points, num_points, mx,my, (x1+x2)/2.0f,(y1+y2)/2.0f, x2,y2, objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points, *num_points,x2,y2);
      *num_points = *num_points+1;
   }
   return 1;
}

static void stbtt__tesselate_cubic(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
   // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
   float dx0 = x1-x0;
   float dy0 = y1-y0;
   float dx1 = x2-x1;
   float dy1 = y2-y1;
   float dx2 = x3-x2;
   float dy2 = y3-y2;
   float dx = x3-x0;
   float dy = y3-y0;
   float longlen = (float) (STBTT_sqrt(dx0*dx0+dy0*dy0)+STBTT_sqrt(dx1*dx1+dy1*dy1)+STBTT_sqrt(dx2*dx2+dy2*dy2));
   float shortlen = (float) STBTT_sqrt(dx*dx+dy*dy);
   float flatness_squared = longlen*longlen-shortlen*shortlen;

   if (n > 16) // 65536 segments on one curve better be enough!
      return;

   if (flatness_squared > objspace_flatness_squared) {
      float x01 = (x0+x1)/2;
      float y01 = (y0+y1)/2;
      float x12 = (x1+x2)/2;
      float y12 = (y1+y2)/2;
      float x23 = (x2+x3)/2;
      float y23 = (y2+y3)/2;

      float xa = (x01+x12)/2;
      float ya = (y01+y12)/2;
      float xb = (x12+x23)/2;
      float yb = (y12+y23)/2;

      float mx = (xa+xb)/2;
      float my = (ya+yb)/2;

      stbtt__tesselate_cubic(points, num_points, x0,y0, x01,y01, xa,ya, mx,my, objspace_flatness_squared,n+1);
      stbtt__tesselate_cubic(points, num_points, mx,my, xb,yb, x23,y23, x3,y3, objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points, *num_points,x3,y3);
      *num_points = *num_points+1;
   }
}

// returns number of contours
static stbtt__point *stbtt_FlattenCurves(stbtt_vertex *vertices, int num_verts, float objspace_flatness, int **contour_lengths, int *num_contours, void *userdata)
{
   stbtt__point *points=0;
   int num_points=0;

   float objspace_flatness_squared = objspace_flatness * objspace_flatness;
   int i,n=0,start=0, pass;

   // count how many "moves" there are to get the contour count
   for (i=0; i < num_verts; ++i)
      if (vertices[i].type == STBTT_vmove)
         ++n;

   *num_contours = n;
   if (n == 0) return 0;

   *contour_lengths = (int *) STBTT_malloc(sizeof(**contour_lengths) * n, userdata);

   if (*contour_lengths == 0) {
      *num_contours = 0;
      return 0;
   }

   // make two passes through the points so we don't need to realloc
   for (pass=0; pass < 2; ++pass) {
      float x=0,y=0;
      if (pass == 1) {
         points = (stbtt__point *) STBTT_malloc(num_points * sizeof(points[0]), userdata);
         if (points == NULL) goto error;
      }
      num_points = 0;
      n= -1;
      for (i=0; i < num_verts; ++i) {
         switch (vertices[i].type) {
            case STBTT_vmove:
               // start the next contour
               if (n >= 0)
                  (*contour_lengths)[n] = num_points - start;
               ++n;
               start = num_points;

               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x,y);
               break;
            case STBTT_vline:
               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x, y);
               break;
            case STBTT_vcurve:
               stbtt__tesselate_curve(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
            case STBTT_vcubic:
               stbtt__tesselate_cubic(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].cx1, vertices[i].cy1,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
         }
      }
      (*contour_lengths)[n] = num_points - start;
   }

   return points;
error:
   STBTT_free(points, userdata);
   STBTT_free(*contour_lengths, userdata);
   *contour_lengths = 0;
   *num_contours = 0;
   return NULL;
}

STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result, float flatness_in_pixels, stbtt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void *userdata)
{
   float scale            = scale_x > scale_y ? scale_y : scale_x;
   int winding_count      = 0;
   int *winding_lengths   = NULL;
   stbtt__point *windings = stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count, userdata);
   if (windings) {
      stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
      STBTT_free(winding_lengths, userdata);
      STBTT_free(windings, userdata);
   }
}

STBTT_DEF void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata)
{
   STBTT_free(bitmap, userdata);
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   int ix0,iy0,ix1,iy1;
   stbtt__bitmap gbm;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);

   if (scale_x == 0) scale_x = scale_y;
   if (scale_y == 0) {
      if (scale_x == 0) {
         STBTT_free(vertices, info->userdata);
         return NULL;
      }
      scale_y = scale_x;
   }

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,&ix1,&iy1);

   // now we get the size
   gbm.w = (ix1 - ix0);
   gbm.h = (iy1 - iy0);
   gbm.pixels = NULL; // in case we error

   if (width ) *width  = gbm.w;
   if (height) *height = gbm.h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;

   if (gbm.w && gbm.h) {
      gbm.pixels = (unsigned char *) STBTT_malloc(gbm.w * gbm.h, info->userdata);
      if (gbm.pixels) {
         gbm.stride = gbm.w;

         stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
      }
   }
   STBTT_free(vertices, info->userdata);
   return gbm.pixels;
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width, height, xoff, yoff);
}

STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
   int ix0,iy0;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
   stbtt__bitmap gbm;

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,0,0);
   gbm.pixels = output;
   gbm.w = out_w;
   gbm.h = out_h;
   gbm.stride = out_stride;

   if (gbm.w && gbm.h)
      stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0,iy0, 1, info->userdata);

   STBTT_free(vertices, info->userdata);
}

STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
   stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, glyph);
}

STBTT_DEF unsigned char *stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y,shift_x,shift_y, stbtt_FindGlyphIndex(info,codepoint), width,height,xoff,yoff);
}

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint)
{
   stbtt_MakeGlyphBitmapSubpixelPrefilter(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, oversample_x, oversample_y, sub_x, sub_y, stbtt_FindGlyphIndex(info,codepoint));
}

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint)
{
   stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, stbtt_FindGlyphIndex(info,codepoint));
}

STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetCodepointBitmapSubpixel(info, scale_x, scale_y, 0.0f,0.0f, codepoint, width,height,xoff,yoff);
}

STBTT_DEF void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint)
{
   stbtt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, codepoint);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

static int stbtt_BakeFontBitmap_internal(unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                stbtt_bakedchar *chardata)
{
   float scale;
   int x,y,bottom_y, i;
   stbtt_fontinfo f;
   f.userdata = NULL;
   if (!stbtt_InitFont(&f, data, offset))
      return -1;
   STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels
   x=y=1;
   bottom_y = 1;

   scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

   for (i=0; i < num_chars; ++i) {
      int advance, lsb, x0,y0,x1,y1,gw,gh;
      int g = stbtt_FindGlyphIndex(&f, first_char + i);
      stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
      stbtt_GetGlyphBitmapBox(&f, g, scale,scale, &x0,&y0,&x1,&y1);
      gw = x1-x0;
      gh = y1-y0;
      if (x + gw + 1 >= pw)
         y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         return -i;
      STBTT_assert(x+gw < pw);
      STBTT_assert(y+gh < ph);
      stbtt_MakeGlyphBitmap(&f, pixels+x+y*pw, gw,gh,pw, scale,scale, g);
      chardata[i].x0 = (stbtt_int16) x;
      chardata[i].y0 = (stbtt_int16) y;
      chardata[i].x1 = (stbtt_int16) (x + gw);
      chardata[i].y1 = (stbtt_int16) (y + gh);
      chardata[i].xadvance = scale * advance;
      chardata[i].xoff     = (float) x0;
      chardata[i].yoff     = (float) y0;
      x = x + gw + 1;
      if (y+gh+1 > bottom_y)
         bottom_y = y+gh+1;
   }
   return bottom_y;
}

STBTT_DEF void stbtt_GetBakedQuad(const stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int opengl_fillrule)
{
   float d3d_bias = opengl_fillrule ? 0 : -0.5f;
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const stbtt_bakedchar *b = chardata + char_index;
   int round_x = STBTT_ifloor((*xpos + b->xoff) + 0.5f);
   int round_y = STBTT_ifloor((*ypos + b->yoff) + 0.5f);

   q->x0 = round_x + d3d_bias;
   q->y0 = round_y + d3d_bias;
   q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
   q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// rectangle packing replacement routines if you don't have stb_rect_pack.h
//

#ifndef STB_RECT_PACK_VERSION

typedef int stbrp_coord;

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                                                                                //
// COMPILER WARNING ?!?!?                                                         //
//                                                                                //
//                                                                                //
// if you get a compile warning due to these symbols being defined more than      //
// once, move #include "stb_rect_pack.h" before #include "stb_truetype.h"         //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
   int width,height;
   int x,y,bottom_y;
} stbrp_context;

typedef struct
{
   unsigned char x;
} stbrp_node;

struct stbrp_rect
{
   stbrp_coord x,y;
   int id,w,h,was_packed;
};

static void stbrp_init_target(stbrp_context *con, int pw, int ph, stbrp_node *nodes, int num_nodes)
{
   con->width  = pw;
   con->height = ph;
   con->x = 0;
   con->y = 0;
   con->bottom_y = 0;
   STBTT__NOTUSED(nodes);
   STBTT__NOTUSED(num_nodes);
}

static void stbrp_pack_rects(stbrp_context *con, stbrp_rect *rects, int num_rects)
{
   int i;
   for (i=0; i < num_rects; ++i) {
      if (con->x + rects[i].w > con->width) {
         con->x = 0;
         con->y = con->bottom_y;
      }
      if (con->y + rects[i].h > con->height)
         break;
      rects[i].x = con->x;
      rects[i].y = con->y;
      rects[i].was_packed = 1;
      con->x += rects[i].w;
      if (con->y + rects[i].h > con->bottom_y)
         con->bottom_y = con->y + rects[i].h;
   }
   for (   ; i < num_rects; ++i)
      rects[i].was_packed = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-AWESOME (tm Ryan Gordon) packing using stb_rect_pack.h. If
// stb_rect_pack.h isn't available, it uses the BakeFontBitmap strategy.

STBTT_DEF int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int pw, int ph, int stride_in_bytes, int padding, void *alloc_context)
{
   stbrp_context *context = (stbrp_context *) STBTT_malloc(sizeof(*context)            ,alloc_context);
   int            num_nodes = pw - padding;
   stbrp_node    *nodes   = (stbrp_node    *) STBTT_malloc(sizeof(*nodes  ) * num_nodes,alloc_context);

   if (context == NULL || nodes == NULL) {
      if (context != NULL) STBTT_free(context, alloc_context);
      if (nodes   != NULL) STBTT_free(nodes  , alloc_context);
      return 0;
   }

   spc->user_allocator_context = alloc_context;
   spc->width = pw;
   spc->height = ph;
   spc->pixels = pixels;
   spc->pack_info = context;
   spc->nodes = nodes;
   spc->padding = padding;
   spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
   spc->h_oversample = 1;
   spc->v_oversample = 1;
   spc->skip_missing = 0;

   stbrp_init_target(context, pw-padding, ph-padding, nodes, num_nodes);

   if (pixels)
      STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels

   return 1;
}

STBTT_DEF void stbtt_PackEnd  (stbtt_pack_context *spc)
{
   STBTT_free(spc->nodes    , spc->user_allocator_context);
   STBTT_free(spc->pack_info, spc->user_allocator_context);
}

STBTT_DEF void stbtt_PackSetOversampling(stbtt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample)
{
   STBTT_assert(h_oversample <= STBTT_MAX_OVERSAMPLE);
   STBTT_assert(v_oversample <= STBTT_MAX_OVERSAMPLE);
   if (h_oversample <= STBTT_MAX_OVERSAMPLE)
      spc->h_oversample = h_oversample;
   if (v_oversample <= STBTT_MAX_OVERSAMPLE)
      spc->v_oversample = v_oversample;
}

STBTT_DEF void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context *spc, int skip)
{
   spc->skip_missing = skip;
}

#define STBTT__OVER_MASK  (STBTT_MAX_OVERSAMPLE-1)

static void stbtt__h_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[STBTT_MAX_OVERSAMPLE];
   int safe_w = w - kernel_width;
   int j;
   STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < h; ++j) {
      int i;
      unsigned int total;
      STBTT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < w; ++i) {
         STBTT_assert(pixels[i] == 0);
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i] = (unsigned char) (total / kernel_width);
      }

      pixels += stride_in_bytes;
   }
}

static void stbtt__v_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[STBTT_MAX_OVERSAMPLE];
   int safe_h = h - kernel_width;
   int j;
   STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < w; ++j) {
      int i;
      unsigned int total;
      STBTT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < h; ++i) {
         STBTT_assert(pixels[i*stride_in_bytes] == 0);
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
      }

      pixels += 1;
   }
}

static float stbtt__oversample_shift(int oversample)
{
   if (!oversample)
      return 0.0f;

   // The prefilter is a box filter of width "oversample",
   // which shifts phase by (oversample - 1)/2 pixels in
   // oversampled space. We want to shift in the opposite
   // direction to counter this.
   return (float)-(oversample - 1) / (2.0f * (float)oversample);
}

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects)
{
   int i,j,k;
   int missing_glyph_added = 0;

   k=0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      ranges[i].h_oversample = (unsigned char) spc->h_oversample;
      ranges[i].v_oversample = (unsigned char) spc->v_oversample;
      for (j=0; j < ranges[i].num_chars; ++j) {
         int x0,y0,x1,y1;
         int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
         int glyph = stbtt_FindGlyphIndex(info, codepoint);
         if (glyph == 0 && (spc->skip_missing || missing_glyph_added)) {
            rects[k].w = rects[k].h = 0;
         } else {
            stbtt_GetGlyphBitmapBoxSubpixel(info,glyph,
                                            scale * spc->h_oversample,
                                            scale * spc->v_oversample,
                                            0,0,
                                            &x0,&y0,&x1,&y1);
            rects[k].w = (stbrp_coord) (x1-x0 + spc->padding + spc->h_oversample-1);
            rects[k].h = (stbrp_coord) (y1-y0 + spc->padding + spc->v_oversample-1);
            if (glyph == 0)
               missing_glyph_added = 1;
         }
         ++k;
      }
   }

   return k;
}

STBTT_DEF void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float *sub_x, float *sub_y, int glyph)
{
   stbtt_MakeGlyphBitmapSubpixel(info,
                                 output,
                                 out_w - (prefilter_x - 1),
                                 out_h - (prefilter_y - 1),
                                 out_stride,
                                 scale_x,
                                 scale_y,
                                 shift_x,
                                 shift_y,
                                 glyph);

   if (prefilter_x > 1)
      stbtt__h_prefilter(output, out_w, out_h, out_stride, prefilter_x);

   if (prefilter_y > 1)
      stbtt__v_prefilter(output, out_w, out_h, out_stride, prefilter_y);

   *sub_x = stbtt__oversample_shift(prefilter_x);
   *sub_y = stbtt__oversample_shift(prefilter_y);
}

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects)
{
   int i,j,k, missing_glyph = -1, return_value = 1;

   // save current values
   int old_h_over = spc->h_oversample;
   int old_v_over = spc->v_oversample;

   k = 0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      float recip_h,recip_v,sub_x,sub_y;
      spc->h_oversample = ranges[i].h_oversample;
      spc->v_oversample = ranges[i].v_oversample;
      recip_h = 1.0f / spc->h_oversample;
      recip_v = 1.0f / spc->v_oversample;
      sub_x = stbtt__oversample_shift(spc->h_oversample);
      sub_y = stbtt__oversample_shift(spc->v_oversample);
      for (j=0; j < ranges[i].num_chars; ++j) {
         stbrp_rect *r = &rects[k];
         if (r->was_packed && r->w != 0 && r->h != 0) {
            stbtt_packedchar *bc = &ranges[i].chardata_for_range[j];
            int advance, lsb, x0,y0,x1,y1;
            int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
            int glyph = stbtt_FindGlyphIndex(info, codepoint);
            stbrp_coord pad = (stbrp_coord) spc->padding;

            // pad on left and top
            r->x += pad;
            r->y += pad;
            r->w -= pad;
            r->h -= pad;
            stbtt_GetGlyphHMetrics(info, glyph, &advance, &lsb);
            stbtt_GetGlyphBitmapBox(info, glyph,
                                    scale * spc->h_oversample,
                                    scale * spc->v_oversample,
                                    &x0,&y0,&x1,&y1);
            stbtt_MakeGlyphBitmapSubpixel(info,
                                          spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                          r->w - spc->h_oversample+1,
                                          r->h - spc->v_oversample+1,
                                          spc->stride_in_bytes,
                                          scale * spc->h_oversample,
                                          scale * spc->v_oversample,
                                          0,0,
                                          glyph);

            if (spc->h_oversample > 1)
               stbtt__h_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->h_oversample);

            if (spc->v_oversample > 1)
               stbtt__v_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->v_oversample);

            bc->x0       = (stbtt_int16)  r->x;
            bc->y0       = (stbtt_int16)  r->y;
            bc->x1       = (stbtt_int16) (r->x + r->w);
            bc->y1       = (stbtt_int16) (r->y + r->h);
            bc->xadvance =                scale * advance;
            bc->xoff     =       (float)  x0 * recip_h + sub_x;
            bc->yoff     =       (float)  y0 * recip_v + sub_y;
            bc->xoff2    =                (x0 + r->w) * recip_h + sub_x;
            bc->yoff2    =                (y0 + r->h) * recip_v + sub_y;

            if (glyph == 0)
               missing_glyph = j;
         } else if (spc->skip_missing) {
            return_value = 0;
         } else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
            ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
         } else {
            return_value = 0; // if any fail, report failure
         }

         ++k;
      }
   }

   // restore original values
   spc->h_oversample = old_h_over;
   spc->v_oversample = old_v_over;

   return return_value;
}

STBTT_DEF void stbtt_PackFontRangesPackRects(stbtt_pack_context *spc, stbrp_rect *rects, int num_rects)
{
   stbrp_pack_rects((stbrp_context *) spc->pack_info, rects, num_rects);
}

STBTT_DEF int stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges)
{
   stbtt_fontinfo info;
   int i,j,n, return_value = 1;
   //stbrp_context *context = (stbrp_context *) spc->pack_info;
   stbrp_rect    *rects;

   // flag all characters as NOT packed
   for (i=0; i < num_ranges; ++i)
      for (j=0; j < ranges[i].num_chars; ++j)
         ranges[i].chardata_for_range[j].x0 =
         ranges[i].chardata_for_range[j].y0 =
         ranges[i].chardata_for_range[j].x1 =
         ranges[i].chardata_for_range[j].y1 = 0;

   n = 0;
   for (i=0; i < num_ranges; ++i)
      n += ranges[i].num_chars;

   rects = (stbrp_rect *) STBTT_malloc(sizeof(*rects) * n, spc->user_allocator_context);
   if (rects == NULL)
      return 0;

   info.userdata = spc->user_allocator_context;
   stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata,font_index));

   n = stbtt_PackFontRangesGatherRects(spc, &info, ranges, num_ranges, rects);

   stbtt_PackFontRangesPackRects(spc, rects, n);

   return_value = stbtt_PackFontRangesRenderIntoRects(spc, &info, ranges, num_ranges, rects);

   STBTT_free(rects, spc->user_allocator_context);
   return return_value;
}

STBTT_DEF int stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
            int first_unicode_codepoint_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range)
{
   stbtt_pack_range range;
   range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
   range.array_of_unicode_codepoints = NULL;
   range.num_chars                   = num_chars_in_range;
   range.chardata_for_range          = chardata_for_range;
   range.font_size                   = font_size;
   return stbtt_PackFontRanges(spc, fontdata, font_index, &range, 1);
}

STBTT_DEF void stbtt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap)
{
   int i_ascent, i_descent, i_lineGap;
   float scale;
   stbtt_fontinfo info;
   stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, index));
   scale = size > 0 ? stbtt_ScaleForPixelHeight(&info, size) : stbtt_ScaleForMappingEmToPixels(&info, -size);
   stbtt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_lineGap);
   *ascent  = (float) i_ascent  * scale;
   *descent = (float) i_descent * scale;
   *lineGap = (float) i_lineGap * scale;
}

STBTT_DEF void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer)
{
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const stbtt_packedchar *b = chardata + char_index;

   if (align_to_integer) {
      float x = (float) STBTT_ifloor((*xpos + b->xoff) + 0.5f);
      float y = (float) STBTT_ifloor((*ypos + b->yoff) + 0.5f);
      q->x0 = x;
      q->y0 = y;
      q->x1 = x + b->xoff2 - b->xoff;
      q->y1 = y + b->yoff2 - b->yoff;
   } else {
      q->x0 = *xpos + b->xoff;
      q->y0 = *ypos + b->yoff;
      q->x1 = *xpos + b->xoff2;
      q->y1 = *ypos + b->yoff2;
   }

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// sdf computation
//

#define STBTT_min(a,b)  ((a) < (b) ? (a) : (b))
#define STBTT_max(a,b)  ((a) < (b) ? (b) : (a))

static int stbtt__ray_intersect_bezier(float orig[2], float ray[2], float q0[2], float q1[2], float q2[2], float hits[2][2])
{
   float q0perp = q0[1]*ray[0] - q0[0]*ray[1];
   float q1perp = q1[1]*ray[0] - q1[0]*ray[1];
   float q2perp = q2[1]*ray[0] - q2[0]*ray[1];
   float roperp = orig[1]*ray[0] - orig[0]*ray[1];

   float a = q0perp - 2*q1perp + q2perp;
   float b = q1perp - q0perp;
   float c = q0perp - roperp;

   float s0 = 0., s1 = 0.;
   int num_s = 0;

   if (a != 0.0) {
      float discr = b*b - a*c;
      if (discr > 0.0) {
         float rcpna = -1 / a;
         float d = (float) STBTT_sqrt(discr);
         s0 = (b+d) * rcpna;
         s1 = (b-d) * rcpna;
         if (s0 >= 0.0 && s0 <= 1.0)
            num_s = 1;
         if (d > 0.0 && s1 >= 0.0 && s1 <= 1.0) {
            if (num_s == 0) s0 = s1;
            ++num_s;
         }
      }
   } else {
      // 2*b*s + c = 0
      // s = -c / (2*b)
      s0 = c / (-2 * b);
      if (s0 >= 0.0 && s0 <= 1.0)
         num_s = 1;
   }

   if (num_s == 0)
      return 0;
   else {
      float rcp_len2 = 1 / (ray[0]*ray[0] + ray[1]*ray[1]);
      float rayn_x = ray[0] * rcp_len2, rayn_y = ray[1] * rcp_len2;

      float q0d =   q0[0]*rayn_x +   q0[1]*rayn_y;
      float q1d =   q1[0]*rayn_x +   q1[1]*rayn_y;
      float q2d =   q2[0]*rayn_x +   q2[1]*rayn_y;
      float rod = orig[0]*rayn_x + orig[1]*rayn_y;

      float q10d = q1d - q0d;
      float q20d = q2d - q0d;
      float q0rd = q0d - rod;

      hits[0][0] = q0rd + s0*(2.0f - 2.0f*s0)*q10d + s0*s0*q20d;
      hits[0][1] = a*s0+b;

      if (num_s > 1) {
         hits[1][0] = q0rd + s1*(2.0f - 2.0f*s1)*q10d + s1*s1*q20d;
         hits[1][1] = a*s1+b;
         return 2;
      } else {
         return 1;
      }
   }
}

static int equal(float *a, float *b)
{
   return (a[0] == b[0] && a[1] == b[1]);
}

static int stbtt__compute_crossings_x(float x, float y, int nverts, stbtt_vertex *verts)
{
   int i;
   float orig[2], ray[2] = { 1, 0 };
   float y_frac;
   int winding = 0;

   orig[0] = x;
   orig[1] = y;

   // make sure y never passes through a vertex of the shape
   y_frac = (float) STBTT_fmod(y, 1.0f);
   if (y_frac < 0.01f)
      y += 0.01f;
   else if (y_frac > 0.99f)
      y -= 0.01f;
   orig[1] = y;

   // test a ray from (-infinity,y) to (x,y)
   for (i=0; i < nverts; ++i) {
      if (verts[i].type == STBTT_vline) {
         int x0 = (int) verts[i-1].x, y0 = (int) verts[i-1].y;
         int x1 = (int) verts[i  ].x, y1 = (int) verts[i  ].y;
         if (y > STBTT_min(y0,y1) && y < STBTT_max(y0,y1) && x > STBTT_min(x0,x1)) {
            float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
            if (x_inter < x)
               winding += (y0 < y1) ? 1 : -1;
         }
      }
      if (verts[i].type == STBTT_vcurve) {
         int x0 = (int) verts[i-1].x , y0 = (int) verts[i-1].y ;
         int x1 = (int) verts[i  ].cx, y1 = (int) verts[i  ].cy;
         int x2 = (int) verts[i  ].x , y2 = (int) verts[i  ].y ;
         int ax = STBTT_min(x0,STBTT_min(x1,x2)), ay = STBTT_min(y0,STBTT_min(y1,y2));
         int by = STBTT_max(y0,STBTT_max(y1,y2));
         if (y > ay && y < by && x > ax) {
            float q0[2],q1[2],q2[2];
            float hits[2][2];
            q0[0] = (float)x0;
            q0[1] = (float)y0;
            q1[0] = (float)x1;
            q1[1] = (float)y1;
            q2[0] = (float)x2;
            q2[1] = (float)y2;
            if (equal(q0,q1) || equal(q1,q2)) {
               x0 = (int)verts[i-1].x;
               y0 = (int)verts[i-1].y;
               x1 = (int)verts[i  ].x;
               y1 = (int)verts[i  ].y;
               if (y > STBTT_min(y0,y1) && y < STBTT_max(y0,y1) && x > STBTT_min(x0,x1)) {
                  float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
                  if (x_inter < x)
                     winding += (y0 < y1) ? 1 : -1;
               }
            } else {
               int num_hits = stbtt__ray_intersect_bezier(orig, ray, q0, q1, q2, hits);
               if (num_hits >= 1)
                  if (hits[0][0] < 0)
                     winding += (hits[0][1] < 0 ? -1 : 1);
               if (num_hits >= 2)
                  if (hits[1][0] < 0)
                     winding += (hits[1][1] < 0 ? -1 : 1);
            }
         }
      }
   }
   return winding;
}

static float stbtt__cuberoot( float x )
{
   if (x<0)
      return -(float) STBTT_pow(-x,1.0f/3.0f);
   else
      return  (float) STBTT_pow( x,1.0f/3.0f);
}

// x^3 + c*x^2 + b*x + a = 0
static int stbtt__solve_cubic(float a, float b, float c, float* r)
{
	float s = -a / 3;
	float p = b - a*a / 3;
	float q = a * (2*a*a - 9*b) / 27 + c;
   float p3 = p*p*p;
	float d = q*q + 4*p3 / 27;
	if (d >= 0) {
		float z = (float) STBTT_sqrt(d);
		float u = (-q + z) / 2;
		float v = (-q - z) / 2;
		u = stbtt__cuberoot(u);
		v = stbtt__cuberoot(v);
		r[0] = s + u + v;
		return 1;
	} else {
	   float u = (float) STBTT_sqrt(-p/3);
	   float v = (float) STBTT_acos(-STBTT_sqrt(-27/p3) * q / 2) / 3; // p3 must be negative, since d is negative
	   float m = (float) STBTT_cos(v);
      float n = (float) STBTT_cos(v-3.141592/2)*1.732050808f;
	   r[0] = s + u * 2 * m;
	   r[1] = s - u * (m + n);
	   r[2] = s - u * (m - n);

      //STBTT_assert( STBTT_fabs(((r[0]+a)*r[0]+b)*r[0]+c) < 0.05f);  // these asserts may not be safe at all scales, though they're in bezier t parameter units so maybe?
      //STBTT_assert( STBTT_fabs(((r[1]+a)*r[1]+b)*r[1]+c) < 0.05f);
      //STBTT_assert( STBTT_fabs(((r[2]+a)*r[2]+b)*r[2]+c) < 0.05f);
   	return 3;
   }
}

STBTT_DEF unsigned char * stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   float scale_x = scale, scale_y = scale;
   int ix0,iy0,ix1,iy1;
   int w,h;
   unsigned char *data;

   if (scale == 0) return NULL;

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale, scale, 0.0f,0.0f, &ix0,&iy0,&ix1,&iy1);

   // if empty, return NULL
   if (ix0 == ix1 || iy0 == iy1)
      return NULL;

   ix0 -= padding;
   iy0 -= padding;
   ix1 += padding;
   iy1 += padding;

   w = (ix1 - ix0);
   h = (iy1 - iy0);

   if (width ) *width  = w;
   if (height) *height = h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;

   // invert for y-downwards bitmaps
   scale_y = -scale_y;

   {
      int x,y,i,j;
      float *precompute;
      stbtt_vertex *verts;
      int num_verts = stbtt_GetGlyphShape(info, glyph, &verts);
      data = (unsigned char *) STBTT_malloc(w * h, info->userdata);
      precompute = (float *) STBTT_malloc(num_verts * sizeof(float), info->userdata);

      for (i=0,j=num_verts-1; i < num_verts; j=i++) {
         if (verts[i].type == STBTT_vline) {
            float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;
            float x1 = verts[j].x*scale_x, y1 = verts[j].y*scale_y;
            float dist = (float) STBTT_sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
            precompute[i] = (dist == 0) ? 0.0f : 1.0f / dist;
         } else if (verts[i].type == STBTT_vcurve) {
            float x2 = verts[j].x *scale_x, y2 = verts[j].y *scale_y;
            float x1 = verts[i].cx*scale_x, y1 = verts[i].cy*scale_y;
            float x0 = verts[i].x *scale_x, y0 = verts[i].y *scale_y;
            float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
            float len2 = bx*bx + by*by;
            if (len2 != 0.0f)
               precompute[i] = 1.0f / (bx*bx + by*by);
            else
               precompute[i] = 0.0f;
         } else
            precompute[i] = 0.0f;
      }

      for (y=iy0; y < iy1; ++y) {
         for (x=ix0; x < ix1; ++x) {
            float val;
            float min_dist = 999999.0f;
            float sx = (float) x + 0.5f;
            float sy = (float) y + 0.5f;
            float x_gspace = (sx / scale_x);
            float y_gspace = (sy / scale_y);

            int winding = stbtt__compute_crossings_x(x_gspace, y_gspace, num_verts, verts); // @OPTIMIZE: this could just be a rasterization, but needs to be line vs. non-tesselated curves so a new path

            for (i=0; i < num_verts; ++i) {
               float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;

               // check against every point here rather than inside line/curve primitives -- @TODO: wrong if multiple 'moves' in a row produce a garbage point, and given culling, probably more efficient to do within line/curve
               float dist2 = (x0-sx)*(x0-sx) + (y0-sy)*(y0-sy);
               if (dist2 < min_dist*min_dist)
                  min_dist = (float) STBTT_sqrt(dist2);

               if (verts[i].type == STBTT_vline) {
                  float x1 = verts[i-1].x*scale_x, y1 = verts[i-1].y*scale_y;

                  // coarse culling against bbox
                  //if (sx > STBTT_min(x0,x1)-min_dist && sx < STBTT_max(x0,x1)+min_dist &&
                  //    sy > STBTT_min(y0,y1)-min_dist && sy < STBTT_max(y0,y1)+min_dist)
                  float dist = (float) STBTT_fabs((x1-x0)*(y0-sy) - (y1-y0)*(x0-sx)) * precompute[i];
                  STBTT_assert(i != 0);
                  if (dist < min_dist) {
                     // check position along line
                     // x' = x0 + t*(x1-x0), y' = y0 + t*(y1-y0)
                     // minimize (x'-sx)*(x'-sx)+(y'-sy)*(y'-sy)
                     float dx = x1-x0, dy = y1-y0;
                     float px = x0-sx, py = y0-sy;
                     // minimize (px+t*dx)^2 + (py+t*dy)^2 = px*px + 2*px*dx*t + t^2*dx*dx + py*py + 2*py*dy*t + t^2*dy*dy
                     // derivative: 2*px*dx + 2*py*dy + (2*dx*dx+2*dy*dy)*t, set to 0 and solve
                     float t = -(px*dx + py*dy) / (dx*dx + dy*dy);
                     if (t >= 0.0f && t <= 1.0f)
                        min_dist = dist;
                  }
               } else if (verts[i].type == STBTT_vcurve) {
                  float x2 = verts[i-1].x *scale_x, y2 = verts[i-1].y *scale_y;
                  float x1 = verts[i  ].cx*scale_x, y1 = verts[i  ].cy*scale_y;
                  float box_x0 = STBTT_min(STBTT_min(x0,x1),x2);
                  float box_y0 = STBTT_min(STBTT_min(y0,y1),y2);
                  float box_x1 = STBTT_max(STBTT_max(x0,x1),x2);
                  float box_y1 = STBTT_max(STBTT_max(y0,y1),y2);
                  // coarse culling against bbox to avoid computing cubic unnecessarily
                  if (sx > box_x0-min_dist && sx < box_x1+min_dist && sy > box_y0-min_dist && sy < box_y1+min_dist) {
                     int num=0;
                     float ax = x1-x0, ay = y1-y0;
                     float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
                     float mx = x0 - sx, my = y0 - sy;
                     float res[3],px,py,t,it;
                     float a_inv = precompute[i];
                     if (a_inv == 0.0) { // if a_inv is 0, it's 2nd degree so use quadratic formula
                        float a = 3*(ax*bx + ay*by);
                        float b = 2*(ax*ax + ay*ay) + (mx*bx+my*by);
                        float c = mx*ax+my*ay;
                        if (a == 0.0) { // if a is 0, it's linear
                           if (b != 0.0) {
                              res[num++] = -c/b;
                           }
                        } else {
                           float discriminant = b*b - 4*a*c;
                           if (discriminant < 0)
                              num = 0;
                           else {
                              float root = (float) STBTT_sqrt(discriminant);
                              res[0] = (-b - root)/(2*a);
                              res[1] = (-b + root)/(2*a);
                              num = 2; // don't bother distinguishing 1-solution case, as code below will still work
                           }
                        }
                     } else {
                        float b = 3*(ax*bx + ay*by) * a_inv; // could precompute this as it doesn't depend on sample point
                        float c = (2*(ax*ax + ay*ay) + (mx*bx+my*by)) * a_inv;
                        float d = (mx*ax+my*ay) * a_inv;
                        num = stbtt__solve_cubic(b, c, d, res);
                     }
                     if (num >= 1 && res[0] >= 0.0f && res[0] <= 1.0f) {
                        t = res[0], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                     if (num >= 2 && res[1] >= 0.0f && res[1] <= 1.0f) {
                        t = res[1], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                     if (num >= 3 && res[2] >= 0.0f && res[2] <= 1.0f) {
                        t = res[2], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                  }
               }
            }
            if (winding == 0)
               min_dist = -min_dist;  // if outside the shape, value is negative
            val = onedge_value + pixel_dist_scale * min_dist;
            if (val < 0)
               val = 0;
            else if (val > 255)
               val = 255;
            data[(y-iy0)*w+(x-ix0)] = (unsigned char) val;
         }
      }
      STBTT_free(precompute, info->userdata);
      STBTT_free(verts, info->userdata);
   }
   return data;
}

STBTT_DEF unsigned char * stbtt_GetCodepointSDF(const stbtt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphSDF(info, scale, stbtt_FindGlyphIndex(info, codepoint), padding, onedge_value, pixel_dist_scale, width, height, xoff, yoff);
}

STBTT_DEF void stbtt_FreeSDF(unsigned char *bitmap, void *userdata)
{
   STBTT_free(bitmap, userdata);
}

//////////////////////////////////////////////////////////////////////////////
//
// font name matching -- recommended not to use this
//

// check if a utf8 string contains a prefix which is the utf16 string; if so return length of matching utf8 string
static stbtt_int32 stbtt__CompareUTF8toUTF16_bigendian_prefix(stbtt_uint8 *s1, stbtt_int32 len1, stbtt_uint8 *s2, stbtt_int32 len2)
{
   stbtt_int32 i=0;

   // convert utf16 to utf8 and compare the results while converting
   while (len2) {
      stbtt_uint16 ch = s2[0]*256 + s2[1];
      if (ch < 0x80) {
         if (i >= len1) return -1;
         if (s1[i++] != ch) return -1;
      } else if (ch < 0x800) {
         if (i+1 >= len1) return -1;
         if (s1[i++] != 0xc0 + (ch >> 6)) return -1;
         if (s1[i++] != 0x80 + (ch & 0x3f)) return -1;
      } else if (ch >= 0xd800 && ch < 0xdc00) {
         stbtt_uint32 c;
         stbtt_uint16 ch2 = s2[2]*256 + s2[3];
         if (i+3 >= len1) return -1;
         c = ((ch - 0xd800) << 10) + (ch2 - 0xdc00) + 0x10000;
         if (s1[i++] != 0xf0 + (c >> 18)) return -1;
         if (s1[i++] != 0x80 + ((c >> 12) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c >>  6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c      ) & 0x3f)) return -1;
         s2 += 2; // plus another 2 below
         len2 -= 2;
      } else if (ch >= 0xdc00 && ch < 0xe000) {
         return -1;
      } else {
         if (i+2 >= len1) return -1;
         if (s1[i++] != 0xe0 + (ch >> 12)) return -1;
         if (s1[i++] != 0x80 + ((ch >> 6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((ch     ) & 0x3f)) return -1;
      }
      s2 += 2;
      len2 -= 2;
   }
   return i;
}

static int stbtt_CompareUTF8toUTF16_bigendian_internal(char *s1, int len1, char *s2, int len2)
{
   return len1 == stbtt__CompareUTF8toUTF16_bigendian_prefix((stbtt_uint8*) s1, len1, (stbtt_uint8*) s2, len2);
}

// returns results in whatever encoding you request... but note that 2-byte encodings
// will be BIG-ENDIAN... use stbtt_CompareUTF8toUTF16_bigendian() to compare
STBTT_DEF const char *stbtt_GetFontNameString(const stbtt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID)
{
   stbtt_int32 i,count,stringOffset;
   stbtt_uint8 *fc = font->data;
   stbtt_uint32 offset = font->fontstart;
   stbtt_uint32 nm = stbtt__find_table(fc, offset, "name");
   if (!nm) return NULL;

   count = ttUSHORT(fc+nm+2);
   stringOffset = nm + ttUSHORT(fc+nm+4);
   for (i=0; i < count; ++i) {
      stbtt_uint32 loc = nm + 6 + 12 * i;
      if (platformID == ttUSHORT(fc+loc+0) && encodingID == ttUSHORT(fc+loc+2)
          && languageID == ttUSHORT(fc+loc+4) && nameID == ttUSHORT(fc+loc+6)) {
         *length = ttUSHORT(fc+loc+8);
         return (const char *) (fc+stringOffset+ttUSHORT(fc+loc+10));
      }
   }
   return NULL;
}

static int stbtt__matchpair(stbtt_uint8 *fc, stbtt_uint32 nm, stbtt_uint8 *name, stbtt_int32 nlen, stbtt_int32 target_id, stbtt_int32 next_id)
{
   stbtt_int32 i;
   stbtt_int32 count = ttUSHORT(fc+nm+2);
   stbtt_int32 stringOffset = nm + ttUSHORT(fc+nm+4);

   for (i=0; i < count; ++i) {
      stbtt_uint32 loc = nm + 6 + 12 * i;
      stbtt_int32 id = ttUSHORT(fc+loc+6);
      if (id == target_id) {
         // find the encoding
         stbtt_int32 platform = ttUSHORT(fc+loc+0), encoding = ttUSHORT(fc+loc+2), language = ttUSHORT(fc+loc+4);

         // is this a Unicode encoding?
         if (platform == 0 || (platform == 3 && encoding == 1) || (platform == 3 && encoding == 10)) {
            stbtt_int32 slen = ttUSHORT(fc+loc+8);
            stbtt_int32 off = ttUSHORT(fc+loc+10);

            // check if there's a prefix match
            stbtt_int32 matchlen = stbtt__CompareUTF8toUTF16_bigendian_prefix(name, nlen, fc+stringOffset+off,slen);
            if (matchlen >= 0) {
               // check for target_id+1 immediately following, with same encoding & language
               if (i+1 < count && ttUSHORT(fc+loc+12+6) == next_id && ttUSHORT(fc+loc+12) == platform && ttUSHORT(fc+loc+12+2) == encoding && ttUSHORT(fc+loc+12+4) == language) {
                  slen = ttUSHORT(fc+loc+12+8);
                  off = ttUSHORT(fc+loc+12+10);
                  if (slen == 0) {
                     if (matchlen == nlen)
                        return 1;
                  } else if (matchlen < nlen && name[matchlen] == ' ') {
                     ++matchlen;
                     if (stbtt_CompareUTF8toUTF16_bigendian_internal((char*) (name+matchlen), nlen-matchlen, (char*)(fc+stringOffset+off),slen))
                        return 1;
                  }
               } else {
                  // if nothing immediately following
                  if (matchlen == nlen)
                     return 1;
               }
            }
         }

         // @TODO handle other encodings
      }
   }
   return 0;
}

static int stbtt__matches(stbtt_uint8 *fc, stbtt_uint32 offset, stbtt_uint8 *name, stbtt_int32 flags)
{
   stbtt_int32 nlen = (stbtt_int32) STBTT_strlen((char *) name);
   stbtt_uint32 nm,hd;
   if (!stbtt__isfont(fc+offset)) return 0;

   // check italics/bold/underline flags in macStyle...
   if (flags) {
      hd = stbtt__find_table(fc, offset, "head");
      if ((ttUSHORT(fc+hd+44) & 7) != (flags & 7)) return 0;
   }

   nm = stbtt__find_table(fc, offset, "name");
   if (!nm) return 0;

   if (flags) {
      // if we checked the macStyle flags, then just check the family and ignore the subfamily
      if (stbtt__matchpair(fc, nm, name, nlen, 16, -1))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  1, -1))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   } else {
      if (stbtt__matchpair(fc, nm, name, nlen, 16, 17))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  1,  2))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   }

   return 0;
}

static int stbtt_FindMatchingFont_internal(unsigned char *font_collection, char *name_utf8, stbtt_int32 flags)
{
   stbtt_int32 i;
   for (i=0;;++i) {
      stbtt_int32 off = stbtt_GetFontOffsetForIndex(font_collection, i);
      if (off < 0) return off;
      if (stbtt__matches((stbtt_uint8 *) font_collection, off, (stbtt_uint8*) name_utf8, flags))
         return off;
   }
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

STBTT_DEF int stbtt_BakeFontBitmap(const unsigned char *data, int offset,
                                float pixel_height, unsigned char *pixels, int pw, int ph,
                                int first_char, int num_chars, stbtt_bakedchar *chardata)
{
   return stbtt_BakeFontBitmap_internal((unsigned char *) data, offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index)
{
   return stbtt_GetFontOffsetForIndex_internal((unsigned char *) data, index);
}

STBTT_DEF int stbtt_GetNumberOfFonts(const unsigned char *data)
{
   return stbtt_GetNumberOfFonts_internal((unsigned char *) data);
}

STBTT_DEF int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset)
{
   return stbtt_InitFont_internal(info, (unsigned char *) data, offset);
}

STBTT_DEF int stbtt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags)
{
   return stbtt_FindMatchingFont_internal((unsigned char *) fontdata, (char *) name, flags);
}

STBTT_DEF int stbtt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2)
{
   return stbtt_CompareUTF8toUTF16_bigendian_internal((char *) s1, len1, (char *) s2, len2);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // STB_TRUETYPE_IMPLEMENTATION


// FULL VERSION HISTORY
//
//   1.19 (2018-02-11) OpenType GPOS kerning (horizontal only), STBTT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) allow user-defined fabs() replacement
//                     fix memory leak if fontsize=0.0
//                     fix warning from duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use alloc userdata for PackFontRanges
//   1.08 (2015-09-13) document stbtt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     allow PackFontRanges to pack and render in separate phases;
//                     fix stbtt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with STBTT_assert() in new rasterizer
//   1.06 (2015-07-14) performance improvements (~35% faster on x86 and x64 on test machine)
//                     also more precise AA rasterizer, except if shapes overlap
//                     remove need for STBTT_sort
//   1.05 (2015-04-15) fix misplaced definitions for STBTT_STATIC
//   1.04 (2015-04-15) typo in example
//   1.03 (2015-04-12) STBTT_STATIC, fix memory leak in new packing, various fixes
//   1.02 (2014-12-10) fix various warnings & compile issues w/ stb_rect_pack, C++
//   1.01 (2014-12-08) fix subpixel position when oversampling to exactly match
//                        non-oversampled; STBTT_POINT_SIZE for packed case only
//   1.00 (2014-12-06) add new PackBegin etc. API, w/ support for oversampling
//   0.99 (2014-09-18) fix multiple bugs with subpixel rendering (ryg)
//   0.9  (2014-08-07) support certain mac/iOS fonts without an MS platformID
//   0.8b (2014-07-07) fix a warning
//   0.8  (2014-05-25) fix a few more warnings
//   0.7  (2013-09-25) bugfix: subpixel glyph bug fixed in 0.5 had come back
//   0.6c (2012-07-24) improve documentation
//   0.6b (2012-07-20) fix a few more warnings
//   0.6  (2012-07-17) fix warnings; added stbtt_ScaleForMappingEmToPixels,
//                        stbtt_GetFontBoundingBox, stbtt_IsGlyphEmpty
//   0.5  (2011-12-09) bugfixes:
//                        subpixel glyph renderer computed wrong bounding box
//                        first vertex of shape can be off-curve (FreeSans)
//   0.4b (2011-12-03) fixed an error in the font baking example
//   0.4  (2011-12-01) kerning, subpixel rendering (tor)
//                    bugfixes for:
//                        codepoint-to-glyph conversion using table fmt=12
//                        codepoint-to-glyph conversion using table fmt=4
//                        stbtt_GetBakedQuad with non-square texture (Zer)
//                    updated Hello World! sample to use kerning and subpixel
//                    fixed some warnings
//   0.3  (2009-06-24) cmap fmt=12, compound shapes (MM)
//                    userdata, malloc-from-userdata, non-zero fill (stb)
//   0.2  (2009-03-11) Fix unsigned/signed char warnings
//   0.1  (2009-03-09) First public release
//

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/


#endif // NMD_GRAPHICS_IMPLEMENTATION
