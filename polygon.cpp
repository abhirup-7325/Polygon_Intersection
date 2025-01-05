#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;
const double EPSILON = 1e-6;

bool areEqual(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

class Point {
public:
    double x, y;

    Point(double x = 0, double y = 0) : x(x), y(y) {}

    bool operator==(const Point& other) const {
        return areEqual(x, other.x) && areEqual(y, other.y);
    }

    void print() const {
        std::cout << "(" << x << ", " << y << ")";
    }
};

class Line {
public:
    double a, b, c;

    Line(const Point& p1, const Point& p2) {
        a = p2.y - p1.y;
        b = p1.x - p2.x;
        c = p2.x * p1.y - p1.x * p2.y;
    }

    bool contains(const Point& p) const {
        return areEqual(a * p.x + b * p.y + c, 0);
    }

    bool intersection(const Line& other, Point& result) const {
        double determinant = a * other.b - other.a * b;
        if (areEqual(determinant, 0)) {
            return false;
        }
        result.x = (b * other.c - other.b * c) / determinant;
        result.y = (other.a * c - a * other.c) / determinant;
        return true;
    }

    void print() const {
        std::cout << a << "x + " << b << "y + " << c << " = 0";
    }
};

class LineSegment {
public:
    Point p1, p2;

    LineSegment(const Point& p1, const Point& p2) : p1(p1), p2(p2) {}

    bool contains(const Point& p) const {
        Line line(p1, p2);

        if (!line.contains(p)) {
            return false;
        }

        return (std::min(p1.x, p2.x) <= p.x && p.x <= std::max(p1.x, p2.x)) &&
               (std::min(p1.y, p2.y) <= p.y && p.y <= std::max(p1.y, p2.y));
    }

    bool intersection(const LineSegment& other, Point& result) const {
        Line line1(p1, p2);
        Line line2(other.p1, other.p2);

        if (!line1.intersection(line2, result)) {
            return false;
        }

        if (contains(result) && other.contains(result)) {
            return true;
        }
        return false;
    }

    void print() const {
        std::cout << "Segment[";
        p1.print();
        std::cout << " - ";
        p2.print();
        std::cout << "]";
    }
};

class Polygon {
public:
    std::vector<Point> vertices;

    Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}

    std::vector<LineSegment> getEdges() const {
        std::vector<LineSegment> edges;
        int n = vertices.size();
        for (int i = 0; i < n; i++) {
            edges.emplace_back(vertices[i], vertices[(i + 1) % n]);
        }
        return edges;
    }

    bool contains(const Point& p) const {
        int count = 0;
        int n = vertices.size();
        for (int i = 0; i < n; i++) {
            Point v1 = vertices[i];
            Point v2 = vertices[(i + 1) % n];

            LineSegment edge(v1, v2);
            if (edge.contains(p)) {
                return true;
            }

            if (areEqual(v1.y, v2.y)) continue;
            if (p.y < std::min(v1.y, v2.y) || p.y > std::max(v1.y, v2.y)) continue;

            double xIntersect = (p.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y) + v1.x;
            if (areEqual(xIntersect, p.x)) return true;
            if (xIntersect > p.x) count++;
        }

        return (count % 2 == 1);
    }

    string classify(const Polygon& other) const {
        auto edges1 = getEdges();
        auto edges2 = other.getEdges();
        
        bool isTouching = false;
        
        for (const auto& edge1 : edges1) {
            for (const auto& vertex : other.vertices) {
                if (edge1.contains(vertex)) {
                    isTouching = true;
                    break;
                }
            }
        }
        
        for (const auto& edge2 : edges2) {
            for (const auto& vertex : vertices) {
                if (edge2.contains(vertex)) {
                    isTouching = true;
                    break;
                }
            }
        }
        
        for (const auto& edge1 : edges1) {
            for (const auto& edge2 : edges2) {
                if (areCollinear(edge1, edge2) && edgesOverlap(edge1, edge2)) {
                    isTouching = true;
                    break;
                }
            }
        }
        
        bool isIntersecting = false;
        for (const auto& edge1 : edges1) {
            for (const auto& edge2 : edges2) {
                Point intersectionPoint;
                if (edge1.intersection(edge2, intersectionPoint)) {
                    if (!edge1.p1.operator==(intersectionPoint) && 
                        !edge1.p2.operator==(intersectionPoint) && 
                        !edge2.p1.operator==(intersectionPoint) && 
                        !edge2.p2.operator==(intersectionPoint)) {
                        isIntersecting = true;
                        break;
                    }
                }
            }
        }
        
        if (isIntersecting) {
            return "Intersecting";
        }
        
        if (isTouching) {
            return "Touching";
        }
        
        bool thisInsideOther = true;
        bool otherInsideThis = true;
        
        for (const auto& vertex : vertices) {
            if (!other.contains(vertex)) {
                thisInsideOther = false;
                break;
            }
        }
        
        for (const auto& vertex : other.vertices) {
            if (!contains(vertex)) {
                otherInsideThis = false;
                break;
            }
        }
        
        if (thisInsideOther || otherInsideThis) {
            return "Disjoint (Enclosed)";
        }
        
        return "Disjoint (Outside)";
    }

    bool areCollinear(const LineSegment& seg1, const LineSegment& seg2) const {
        Line line1(seg1.p1, seg1.p2);
        return line1.contains(seg2.p1) && line1.contains(seg2.p2);
    }

    bool edgesOverlap(const LineSegment& seg1, const LineSegment& seg2) const {
        auto isBetween = [](double a, double b, double c) {
            return std::min(a, b) <= c && c <= std::max(a, b);
        };

        return (isBetween(seg1.p1.x, seg1.p2.x, seg2.p1.x) ||
                isBetween(seg1.p1.x, seg1.p2.x, seg2.p2.x) ||
                isBetween(seg2.p1.x, seg2.p2.x, seg1.p1.x) ||
                isBetween(seg2.p1.x, seg2.p2.x, seg1.p2.x)) &&
               (isBetween(seg1.p1.y, seg1.p2.y, seg2.p1.y) ||
                isBetween(seg1.p1.y, seg1.p2.y, seg2.p2.y) ||
                isBetween(seg2.p1.y, seg2.p2.y, seg1.p1.y) ||
                isBetween(seg2.p1.y, seg2.p2.y, seg1.p2.y));
    }

    void print() const {
        std::cout << "Polygon: ";
        for (const auto& vertex : vertices) {
            vertex.print();
            std::cout << " ";
        }
        std::cout << "\n";
    }
};

int main() {
    Polygon polygon1({Point(4, 4), Point(4, -4), Point(-4, -4), Point(-4, 4)});
    Polygon polygon2({Point(2, 2), Point(2, -2), Point(-2, -2), Point(2, -2)});

    polygon1.print();
    polygon2.print();

    std::string relationship = polygon1.classify(polygon2);
    std::cout << "Relationship: " << relationship << "\n";

    return 0;
}
