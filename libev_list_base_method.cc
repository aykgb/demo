#include <iostream>
#include <vector>

#include <event2/event.h>
#include <event2/bufferevent.h>

using namespace std;

struct event_base *base;

int main()
{
    base = event_base_new();

    return 0;
}
