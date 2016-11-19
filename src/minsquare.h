#ifndef __MINSQUARE_H__
#define __MINSQUARE_H__

#include <vector>

template<typename T>
class MinSquare
{
public:
    MinSquare();
    T getY(T X);
    void addPoint(std::pair<T, T> point);
private:
    std::vector<T> points;
    T a, b;
    T xSum, ySum, xySum, x2Sum;
    std::size_t n;
};

template<typename T>
MinSquare<T>::MinSquare() :
    a(0), b(0),
    xSum(0), ySum(0), xySum(0), x2Sum(0),
    n(0)
{
}

template<typename T>
void MinSquare<T>::addPoint(std::pair<T, T> point)
{
    ++n;
    xSum += point.first;
    ySum += point.second;
    xySum += point.first * point.second;
    x2Sum += point.first * point.first;

    if (n > 1)
    {
        a = (n * xySum - xSum * ySum) / (n * x2Sum - xSum * xSum);
        b = (ySum - a * xSum) / n;
    }
}

template<typename T>
T MinSquare<T>::getY(T X)
{
    return a * X + b;
}

#endif // __MINSQUARE_H__
