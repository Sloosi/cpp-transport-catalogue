#include "svg.h"

namespace svg {

    using namespace std::literals;

    namespace {

        template <typename T>
        inline void RenderValue(std::ostream& out, const T& value) {
            out << value;
        }

        void HtmlEncodeString(std::ostream& out, std::string_view sv) {
            for (char c : sv) {
                switch (c) {
                case '"':
                    out << "&quot;"sv;
                    break;
                case '<':
                    out << "&lt;"sv;
                    break;
                case '>':
                    out << "&gt;"sv;
                    break;
                case '&':
                    out << "&amp;"sv;
                    break;
                case '\'':
                    out << "&apos;"sv;
                    break;
                default:
                    out.put(c);
                }
            }
        }

        template <>
        void RenderValue<std::string>(std::ostream& out, const std::string& s) {
            HtmlEncodeString(out, s);
        }

        template <typename AttrType>
        void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
            using namespace std::literals;
            out << name << "=\""sv;
            RenderValue(out, value);
            out.put('"');
        }

    }  // namespace

    std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
        switch (value) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
        switch (value) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тэга своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const Point& p : points_) {
            if (first) {
                first = false;
            }
            else {
                out << ' ';
            }
            out << p.x << ',' << p.y;
        }
        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttr(out, " x"sv, position_.x);
        RenderAttr(out, " y"sv, position_.y);
        RenderAttr(out, " dx"sv, offset_.x);
        RenderAttr(out, " dy"sv, offset_.y);
        RenderAttr(out, " font-size"sv, font_size_);
        if (!font_family_.empty()) {
            RenderAttr(out, " font-family"sv, font_family_);
        }
        if (!font_weight_.empty()) {
            RenderAttr(out, " font-weight"sv, font_weight_);
        }
        RenderAttrs(out);
        out.put('>');
        HtmlEncodeString(out, data_);
        out << "</text>"sv;
    }

    // Document

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx{ out, 2, 2 };
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg