#include "silhouette.h"

Silhouette::Silhouette() :
    updated(false)
{
}

bool Silhouette::getUpdated() const
{
    return updated;
}

void Silhouette::setUpdated(bool value)
{
    updated = value;
}

int Silhouette::distanceFrom(const Rect &rect) const
{
    // Centers of the rectangles
    cv::Point c1(     rect.x          +        rect.width/2       ,
                      rect.y          +        rect.height/2      );
    cv::Point c2(previousPos.back().x + previousPos.back().width/2,
                 previousPos.back().y + previousPos.back().height/2);

    return (int)sqrt((c2.x - c1.x)*(c2.x - c1.x) + (c2.y - c1.y)*(c2.y - c1.y));
}

void Silhouette::addPos(const Rect &newPos)
{
    previousPos.push_back(newPos);

    // Delete first if list too long
}
