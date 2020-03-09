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