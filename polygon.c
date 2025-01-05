#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define EPSILON 1e-6

// Point structure
typedef struct {
    double x, y;
} Point;

// Line structure
typedef struct {
    double a, b, c;
} Line;

// Line segment structure
typedef struct {
    Point p1, p2;
} LineSegment;

// Polygon structure
typedef struct {
    Point* vertices;
    int size;
} Polygon;

// Utility functions
bool areEqual(double a, double b) {
    return fabs(a - b) < EPSILON;
}

bool pointsEqual(Point p1, Point p2) {
    return areEqual(p1.x, p2.x) && areEqual(p1.y, p2.y);
}

void printPoint(Point p) {
    printf("(%f, %f)", p.x, p.y);
}

// Line functions
Line createLine(Point p1, Point p2) {
    Line line;
    line.a = p2.y - p1.y;
    line.b = p1.x - p2.x;
    line.c = p2.x * p1.y - p1.x * p2.y;
    return line;
}

bool lineContains(Line line, Point p) {
    return areEqual(line.a * p.x + line.b * p.y + line.c, 0);
}

bool lineIntersection(Line line1, Line line2, Point* result) {
    double determinant = line1.a * line2.b - line2.a * line1.b;
    if (areEqual(determinant, 0)) {
        return false;
    }
    result->x = (line1.b * line2.c - line2.b * line1.c) / determinant;
    result->y = (line2.a * line1.c - line1.a * line2.c) / determinant;
    return true;
}

// Line segment functions
bool segmentContains(LineSegment seg, Point p) {
    Line line = createLine(seg.p1, seg.p2);
    
    if (!lineContains(line, p)) {
        return false;
    }
    
    return (fmin(seg.p1.x, seg.p2.x) <= p.x && p.x <= fmax(seg.p1.x, seg.p2.x)) &&
           (fmin(seg.p1.y, seg.p2.y) <= p.y && p.y <= fmax(seg.p1.y, seg.p2.y));
}

bool segmentIntersection(LineSegment seg1, LineSegment seg2, Point* result) {
    Line line1 = createLine(seg1.p1, seg1.p2);
    Line line2 = createLine(seg2.p1, seg2.p2);
    
    if (!lineIntersection(line1, line2, result)) {
        return false;
    }
    
    return segmentContains(seg1, *result) && segmentContains(seg2, *result);
}

// Helper functions for polygon
LineSegment* getEdges(const Polygon* poly) {
    LineSegment* edges = malloc(poly->size * sizeof(LineSegment));
    for (int i = 0; i < poly->size; i++) {
        edges[i].p1 = poly->vertices[i];
        edges[i].p2 = poly->vertices[(i + 1) % poly->size];
    }
    return edges;
}

bool areCollinear(LineSegment seg1, LineSegment seg2) {
    Line line = createLine(seg1.p1, seg1.p2);
    return lineContains(line, seg2.p1) && lineContains(line, seg2.p2);
}

bool edgesOverlap(LineSegment seg1, LineSegment seg2) {
    bool isBetween(double a, double b, double c) {
        return fmin(a, b) <= c && c <= fmax(a, b);
    }
    
    return (isBetween(seg1.p1.x, seg1.p2.x, seg2.p1.x) ||
            isBetween(seg1.p1.x, seg1.p2.x, seg2.p2.x) ||
            isBetween(seg2.p1.x, seg2.p2.x, seg1.p1.x) ||
            isBetween(seg2.p1.x, seg2.p2.x, seg1.p2.x)) &&
           (isBetween(seg1.p1.y, seg1.p2.y, seg2.p1.y) ||
            isBetween(seg1.p1.y, seg1.p2.y, seg2.p2.y) ||
            isBetween(seg2.p1.y, seg2.p2.y, seg1.p1.y) ||
            isBetween(seg2.p1.y, seg2.p2.y, seg1.p2.y));
}

bool polygonContains(const Polygon* poly, Point p) {
    int count = 0;
    for (int i = 0; i < poly->size; i++) {
        Point v1 = poly->vertices[i];
        Point v2 = poly->vertices[(i + 1) % poly->size];
        LineSegment edge = {v1, v2};
        
        if (segmentContains(edge, p)) {
            return true;
        }
        
        if (areEqual(v1.y, v2.y)) continue;
        if (p.y < fmin(v1.y, v2.y) || p.y > fmax(v1.y, v2.y)) continue;
        
        double xIntersect = (p.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y) + v1.x;
        if (areEqual(xIntersect, p.x)) return true;
        if (xIntersect > p.x) count++;
    }
    
    return (count % 2 == 1);
}

char* classifyPolygons(const Polygon* poly1, const Polygon* poly2) {
    LineSegment* edges1 = getEdges(poly1);
    LineSegment* edges2 = getEdges(poly2);
    bool isTouching = false;
    bool isIntersecting = false;
    
    for (int i = 0; i < poly1->size; i++) {
        for (int j = 0; j < poly2->size; j++) {
            if (segmentContains(edges1[i], poly2->vertices[j])) {
                isTouching = true;
            }
            
            if (areCollinear(edges1[i], edges2[j]) && edgesOverlap(edges1[i], edges2[j])) {
                isTouching = true;
            }
            
            Point intersectionPoint;
            if (segmentIntersection(edges1[i], edges2[j], &intersectionPoint)) {
                if (!pointsEqual(intersectionPoint, edges1[i].p1) && 
                    !pointsEqual(intersectionPoint, edges1[i].p2) && 
                    !pointsEqual(intersectionPoint, edges2[j].p1) && 
                    !pointsEqual(intersectionPoint, edges2[j].p2)) {
                    isIntersecting = true;
                }
            }
        }
    }
    
    for (int i = 0; i < poly2->size; i++) {
        for (int j = 0; j < poly1->size; j++) {
            if (segmentContains(edges2[i], poly1->vertices[j])) {
                isTouching = true;
            }
        }
    }
    
    free(edges1);
    free(edges2);
    
    if (isIntersecting) {
        return "Intersecting";
    }
    
    if (isTouching) {
        return "Touching";
    }
    
    bool thisInsideOther = true;
    bool otherInsideThis = true;
    
    for (int i = 0; i < poly1->size; i++) {
        if (!polygonContains(poly2, poly1->vertices[i])) {
            thisInsideOther = false;
            break;
        }
    }
    
    for (int i = 0; i < poly2->size; i++) {
        if (!polygonContains(poly1, poly2->vertices[i])) {
            otherInsideThis = false;
            break;
        }
    }
    
    if (thisInsideOther || otherInsideThis) {
        return "Disjoint (Enclosed)";
    }
    
    return "Disjoint (Outside)";
}

void printPolygon(const Polygon* poly) {
    printf("Polygon: ");
    for (int i = 0; i < poly->size; i++) {
        printPoint(poly->vertices[i]);
        printf(" ");
    }
    printf("\n");
}


int main() {
    Point vertices1[] = {
        {4, 4}, {4, -4}, {-4, -4}, {-4, 4}
    };
    Polygon poly1 = {vertices1, 4};
    
    Point vertices2[] = {
        {4, 4}, {2, -2}, {-2, -2}, {2, -2}
    };
    Polygon poly2 = {vertices2, 4};
    
    printPolygon(&poly1);
    printPolygon(&poly2);
    
    char* relationship = classifyPolygons(&poly1, &poly2);
    printf("Relationship: %s\n", relationship);
    
    return 0;
}
