# Blacksmith

A c++17 single header library for constructing and building classes inline.

## Overview

Initialize a class and modify it in-place:

```cpp
// Returns a Label with size: 200 x 50, and text: "Ok"
build([](Label& _){ _.set_size(200, 50); }, "Ok");
```

Or directly create a new raw or smart pointer
```cpp
// Returns a new Label*
build_new([](Label& _){...}, "Ok");

// Returns a std::shared_ptr<Label>
build_shared([](Label& _){...}, "Ok");

// Returns a std::unique_ptr<Label>
build_unique([](Label& _){...}, "Ok");
```


## Why?

We want to construct a widget with a button inside.

Standard approach:
```cpp
auto widget = new Widget();
widget->set_color({255, 0, 0});

auto btn_add = new Button("+");
btn_add->set_action(Action::Add);
widget->add_child(button1);

auto btn_subtract = new Button("-");
btn_add->set_action(Action::Subtract);
widget->add_child(btn_subtract);
```

**Blacksmith** approach:
```cpp
auto widget = build_new([](Widget& _) { 
    _.set_color({255, 0, 0}); 
    _.add_child(build_new([](Button& _) {
        _.set_action(Action::Add);
    }, "+"));
    _.add_child(build_new([](Button& _) {
        _.action = Action::Subtract;
    }, "-"));
});
```

Did you spot the mistake in the first approach? This one is hard to detect later on!

```cpp
    auto btn_subtract = new Button("-");
    btn_add->set_action(Action::Subtract);
//  ^^^^^^^ this should be btn_subtract
    widget->add_child(btn_subtract);
```

**Blacksmith** aims to eliminate this type of error by encouraging a more declarative style of programming.



## Features


* Only write the type once! And don't worry about thinking of a unique variable name.
```cpp
build([](Widget& _) { ... });
         ^^^^^^  ^
```

* Trailing arguments are forwarded to the constructor
```cpp
build([](Label& _) { ... }, "Hello, world");
                            ^^^^^^^^^^^^^^
```

* All `build` functions are `constexpr` and are optimized away by the compiler.

* Build functions can accept a reference or the actual type
```cpp
build_shared([](Label& _) {...}); // Ok
                ^^^^^^
build_shared([](const std::shared_ptr<Label>& _) {...}); // Ok
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```


# Examples

## Comparison of GUI Construction

### Without Blacksmith
```cpp
auto layout = std::make_shared<Layout>(LayoutDirection::Vertical);
auto outer_widget = std::make_shared<Widget>();
outer_widget->set_background({255, 255, 255});
outer_widget->set_padding(4);
outer_widget->set_size(100, 100);

auto inner_layout = std::make_shared<Layout>(LayoutDirection::Horizontal);

auto inner_label = std::make_shared<Label>("Red Box");
inner_label->set_background({255, 255, 255});
inner_label->set_size(300, 50);
inner_layout->add_child(inner_label);

auto inner_box = std::make_shared<Widget>();
inner_box->set_background({255, 0, 0});
inner_box->set_size(50, 50);

outer_widget->set_layout(inner_layout);

layout->add_child(outer_widget);
draw_layout(layout);
```

### With Blacksmith
```cpp
using namespace blacksmith;
draw_layout(build_shared(
    [](Layout &_) {
        _.add_child(build_shared([](Widget &_) {
            _.set_background({255, 255, 255});
            _.set_padding(4);
            _.set_size(100, 100);
            _.set_layout(build_shared(
                [](Layout &_) {
                    _.add_child(build_shared(
                        [](Label &_) {
                            _.set_background({255, 255, 255});
                            _.set_size(300, 50);
                        },
                        "Red Box"));
                    _.add_child(build_shared([](Widget &_) {
                        _.set_background({255, 0, 0});
                        _.set_size(50, 50);
                    }));
                },
                LayoutDirection::Horizontal));
        }));
    },
    LayoutDirection::Vertical));
```



## Usage of all build functions

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