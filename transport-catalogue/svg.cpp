#include "svg.h"

namespace svg {

using namespace std::literals;

struct OstreamPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none"sv;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb color) const {
        out << "rgb("sv << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ')';
    }
    void operator()(Rgba color) const {
        out << "rgba("sv << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ',' << color.alpha << ')';
    }
};

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(OstreamPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream &out, const StrokeLineCap& stroke_linecap) {
    switch (stroke_linecap) {
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

std::ostream &operator<<(std::ostream &out, const StrokeLineJoin& stroke_linejoin) {
    switch (stroke_linejoin) {
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

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
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
    for (const auto& point : points_) {
        if (!first) {
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        first = false;
    }
    out << "\"";
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << " x=\"" << pos_.x << "\""
        << " y=\"" << pos_.y << "\""
        << " dx=\"" << offset_.x << "\""
        << " dy=\"" << offset_.y << "\""
        << " font-size=\"" << size_ << "\"";

    if (font_family_ != "") {
        out << " font-family=\"" << font_family_ << "\"";
    }

    if (font_weight_ != "") {
        out << " font-weight=\"" << font_weight_ << "\"";
    }

    out << ">";

    for (const auto ch : data_) {
        if (ch == '"') {
            out << "&quot;";
            continue;
        }

        if (ch == '\'') {
            out << "&apos;";
            continue;
        }

        if (ch == '<') {
            out << "&lt;";
            continue;
        }

        if (ch == '>') {
            out << "&gt;";
            continue;
        }

        if (ch == '&') {
            out << "&amp;";
            continue;
        }

        out << ch;
    }

    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out, 0, 4));
    }
    out << "</svg>"sv;
}

}  // namespace svg

