# blacksmith

A c++17 single header library for constructing and building classes inline.

## Overview

Build a local class:
```cpp
Point p = build([](Point& _) { _.x = 5; _.y = 6;});
```

Construct a `shared_ptr`, `unique_ptr` or raw pointer:
```cpp
std::shared_ptr<Point> p = build_shared([](Point& _) { _.x = 5; _.y = 6;});
std::unique<Point> p = build_unique([](Point& _) { _.x = 5; _.y = 6;});
Point* p = build_new([](Point& _) { _.x = 5; _.y = 6;}); // ... delete point;
```

All `build` functions are `constexpr` and are optimised away by the compiler.

## Why?

```cpp
Point point1;
point1.setX(1);
point1.setY(2);
point1.setZ(3);

Point point2;
point2.setX(4);
point1.setY(5); // <-- Oops!! See what we did here?
point2.setZ(6);

drawLine(point1, point2);
```

If we use *blacksmith* instead:
```cpp
drawLine(build([](Point&_) { _.setX(1), _.setY(2), _.setZ(3); }),
            build([](Point&_) { _.setX(4), _.setY(5), _.setZ(6); }));
```
By using the `build` function, we avoided the possibility of accidentally setting the wrong named variable!
All variables are now scoped locally and constructed in-place!


## Example

```cpp
#include <string>
#include <vector>
#include "blacksmith.h"

struct Pet
{
    std::string species;
    std::string name;
    std::string full_name() const { return name + " " + species; }
};

struct Person
{
    std::string first_name;
    std::string last_name;
    int age;
    std::vector<Pet> local_pets;
    std::vector<std::shared_ptr<Pet>> shared_pets;
    std::vector<std::shared_ptr<Pet>> unique_pets;
    std::vector<Pet *> raw_owned_pets;
    ~Person()
    {
        for (auto *pet : raw_owned_pets) {
            delete pet;
        }
    }
};

int main()
{
    using namespace blacksmith;
    auto owner = build([](Person &_) {
        _.first_name = "Jon";
        _.last_name = "Doe";
        _.age = 42;
        _.local_pets = {{
            build([](Pet &_) {
                _.species = "Bat";
                _.name = "Cricket";
            }),
        }};
        _.shared_pets = {{
            // Argument can be a reference
            build_shared([](Pet &_) {
                _.species = "Cat";
                _.name = "Smelly";
            }),
            // Or a shared pointer
            build_shared([](const std::shared_ptr<Pet> &_) {
                _->species = "Dog";
                _->name = "Hot";
            }),
        }};
        _.unique_pets = {{
            build_unique([](Pet &_) {
                _.species = "Cat";
                _.name = "Copy";
            }),
            build_unique([](std::unique_ptr<Pet> &_) {
                _->species = "Cat";
                _->name = "Bob";
            }),
        }};
        _.raw_owned_pets = {{
            build_new([](Pet &_) {
                _.species = "Chicken";
                _.name = "Arya";
            }),
            build_new([](Pet *_) {
                _->species = "Otter";
                _->name = "Hairy";
            }),
        }};
    });
```