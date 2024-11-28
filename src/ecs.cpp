#include "ecs.h"
#include <iostream>

void meh() {}


EntityID EntityManager::newEntity(std::string name)
{
    EntityID newID = nextEntityID++;
    entityNames.insert({newID, name});
    //std::cout << __func__ << "(" << name << ") -> " << newID << "\n";
    return newID;
}

Components &EntityManager::getComponents(EntityID id)
{
    if ( entityComponentMap.find(id) == entityComponentMap.end() )
    {
        entityComponentMap[id] = Components();
    }
    //std::cout << __func__ << "(" << id << ") -> " << entityComponentMap[id] << "\n";
    return entityComponentMap[id];
}

SystemID EntityManager::newSystem(std::string name)
{
    SystemID newID = nextSystemID++;
    //std::cerr << "add new (name="<< name << ") system with id " << newID << "\n";
    systemNames.insert({newID, name});
    //std::cout << __func__ << "(" << name << ") -> " << newID << "\n";
    return newID;
}

SystemID EntityManager::addSystem(Components components, std::string name)
{
    SystemID system = nextSystemID++;
    //std::cerr << "add new ("<< name <<") system with id " << system << "\n";
    if ( systemComponnentMap.find(system) == systemComponnentMap.end() )
    {
        systemComponnentMap.insert({system, components});
        systemEntities.insert({system,{}});
        systemNames.insert({system, name});
    }
    else
    {
        systemComponnentMap[system] = components;
    }
    updateSystem(system);
    return system;
}

bool EntityManager::updateSystemComponents(SystemID system, Components components)
{
    //std::cerr << "update components of system " << system << "\n";
    systemComponnentMap[system] = components;
    updateSystem(system);
    return true;
}

void EntityManager::updateSystem(SystemID system)
{
    Components systemComponents = systemComponnentMap[system];
    systemEntities[system].clear();
    for ( auto components : entityComponentMap )
    {
        Components entityComponents = components.second;
        if ( (entityComponents & systemComponents) == systemComponents )
        {
            systemEntities[system].push_back(components.first);
        }
    }
    //std::cerr << "system " << system << " has " << systemEntities[system].size() << " target entities\n";
}

void EntityManager::showAll()
{
    std::cout << "Entities:\n";
    for ( auto entity : entityComponentMap )
    {
        std::cout << "\t" << entityNames[entity.first] << ": " << entity.first << " -> " << entity.second << "\n";
        for ( auto cmp : entityComponentIndices[entity.first] )
        {
            std::cout << "\t\t" << cmp.first << " -> " << cmp.second << "\n";
        }
    }
    std::cout << "Components:\n";
    for ( auto component : ComponentIDMap )
    {
        std::cout << "\t" << component.first << " -> " << component.second << "\n";
        if ( component.second )
        {
            entityComponents[component.second]->print("\t\t\t");
        }
    }
    std::cout << "Systems:\n";
    for ( auto system : systemComponnentMap )
    {
        std::cout << "\t" << systemNames[system.first] << " " << system.first << " -> " << system.second << "\n";
        for ( auto entity : systemEntities[system.first] )
        {
            std::cout << "\t\t" << entity << "\n";
        }
    }
}