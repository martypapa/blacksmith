#include <deque>
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

void pets_example()
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

    assert(owner.age == 42);
    build_on(owner, [](Person &_) { _.age = 43; });
    assert(owner.age == 43);
    build_on(&owner, [](Person *_) { _->age = 44; });
    assert(owner.age == 44);
    build_on(&owner, [](Person &_) { _.age = 45; });
    assert(owner.age == 45);
    assert(owner.first_name == "Jon");
    assert(owner.last_name == "Doe");
    assert(owner.local_pets[0].full_name() == "Cricket Bat");
    assert(owner.shared_pets[0]->full_name() == "Smelly Cat");
    assert(owner.shared_pets[1]->full_name() == "Hot Dog");
    assert(owner.unique_pets[0]->full_name() == "Copy Cat");
    assert(owner.unique_pets[1]->full_name() == "Bob Cat");
    assert(owner.raw_owned_pets[0]->full_name() == "Arya Chicken");
    assert(owner.raw_owned_pets[1]->full_name() == "Hairy Otter");
}

enum class LayoutDirection
{
    Vertical = 0,
    Horizontal = 1,
};
class Widget;

class Layout
{
    LayoutDirection direction_;
    std::deque<std::shared_ptr<Widget>> children_;

public:
    Layout(LayoutDirection direction) : direction_(direction) {}
    void add_child(std::shared_ptr<Widget> child) { children_.push_back(child); }
};
class Color
{
    char red_, green_, blue_;

public:
    Color(int red, int green, int blue) : red_(red), green_(green), blue_(blue) {}
};

class Widget
{
    std::shared_ptr<Layout> layout_;
    Color bg_color_{0, 0, 0};
    int width_;
    int height_;
    int padding_;

public:
    Widget() {}
    void set_layout(std::shared_ptr<Layout> layout) { layout_ = layout; }
    void set_background(Color color) { bg_color_ = color; }
    void set_size(int width, int height)
    {
        width_ = width;
        height_ = height;
    }
    void set_padding(int padding) { padding_ = padding; }
};

class Label : public Widget
{
    std::string text_;

public:
    Label(std::string text) : text_(std::move(text)) {}
};

void draw_layout(std::shared_ptr<Layout> layout)
{ /* unimplemented... */
}

void gui_with_blacksmith()
{
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

}
void gui_without_blacksmith()
{
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
}

int main()
{
    pets_example();
    gui_with_blacksmith();
    gui_without_blacksmith();
}