#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
using namespace std;

template <typename Iterator>
class IteratorRange {
public:
  IteratorRange(Iterator first, Iterator last) : first(first), last(last) {
  }

  Iterator begin() const {
    return first;
  }

  Iterator end() const {
    return last;
  }

  size_t size() const {
    return last - first;
  }

private:
  Iterator first, last;
};

template <typename Container>
auto Head(Container& c, int top) {
  return IteratorRange(begin(c), begin(c) + min<size_t>(max(top, 0), c.size()));
}

template <typename Iterator>
class Paginator {
public:
  Paginator(Iterator begin, Iterator end, size_t page_size)
  {
    size_t page_count = ceil((float)(end - begin) / page_size);
    for (int i = 0; i < page_count; i++)
    {
      if (i == page_count - 1)
      {
        pages.push_back(IteratorRange{begin + page_size * i , end});
      }
      else
      {
        pages.push_back(IteratorRange{begin + page_size * i, begin + page_size * (i + 1)});
      }
    }
  }
    
  auto begin() const
  {
    return pages.begin();
  }
  auto end() const
  {
    return pages.end();
  }
  size_t size() const
  {
    return pages.size();
  }
private:
  vector<IteratorRange<Iterator>> pages;
};

template <typename C>
auto Paginate(C& c, size_t page_size) {
  // Реализуйте этот шаблон функции
  return Paginator{c.begin(), c.end(), page_size};
}