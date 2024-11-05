#ifndef _ECS_H_
#define _ECS_H_
#include <bitset>
#include <typeindex>
#include <memory>
#include <array>
#include <map>
#include <vector>
#include <iostream>

typedef uint32_t EntityID;
typedef uint32_t ComponentID;
typedef uint32_t SystemID;

const EntityID SYSTEM = 0;

#define MAX_NUM_COMPONENTS 1000u
typedef std::bitset<MAX_NUM_COMPONENTS> Components;

typedef const char *ComponentType;
typedef uint32_t EntityComponentIndex;

class IComponentList
{
public:
    virtual ~IComponentList() = default;
    virtual void print(const std::string prefix) = 0;
};

template <typename T>
class ComponentList : public IComponentList
{
private:
    std::array<T, MAX_NUM_COMPONENTS> components;
    uint32_t nextIdx;

public:
    virtual ~ComponentList() = default;
    ComponentList() : nextIdx(0) {}
    ComponentList(T firstEntry) : nextIdx(0)
    {
        //std::cout << __func__ << "(" << &firstEntry << ")\n";
        add(firstEntry);
    }
    uint32_t add(T c)
    {
        //std::cout << __func__ << "(" << &c << ")\n";
        //std::cout << "add element to ComponentList<" << typeid(c).name() << "> and size " << sizeof(T) << " at index " << nextIdx << "\n";
        components[nextIdx] = c;
        return nextIdx++;
    }
    T &get(uint32_t idx)
    {
        return components[idx];
    }
    virtual void print(const std::string prefix)
    {
        for ( uint32_t idx = 0; idx < nextIdx; ++idx )
        {
            std::cout << prefix << &(components[idx]) << " : " << "\n";
        }
    }
};

class EntityManager
{
private:
    std::map<EntityID, std::string> entityNames;
    EntityID nextEntityID;
    std::map<ComponentType, ComponentID> ComponentIDMap {};
    ComponentID nextComponentID;

    std::map<EntityID, Components> entityComponentMap {};
    std::map<ComponentID, EntityID> componentToEntityMap {};
    std::map<ComponentID, std::shared_ptr<IComponentList>> entityComponents {};
    std::map<EntityID, std::map<ComponentID, EntityComponentIndex>> entityComponentIndices {};
    
    std::map<SystemID, std::string> systemNames;
    std::map<SystemID, Components> systemComponnentMap {};
    std::map<SystemID, std::vector<EntityID>> systemEntities {};
    SystemID nextSystemID;
public:
    static EntityManager &getInstance()
    {
        static EntityManager singleton;
        return singleton;
    }
    EntityManager() : nextEntityID(SYSTEM+1), nextComponentID(0), nextSystemID(0) {}
    EntityID newEntity(std::string name);
    
    const std::string getEntityName(EntityID id)
    {
        return entityNames[id];
    }

    template <typename T>
    ComponentID registerComponent()
    {
        ComponentType id = ComponentType(typeid(T).name());
        if ( ComponentIDMap.find(id) == ComponentIDMap.end() )
        {
            ComponentIDMap[id] = nextComponentID++;
            entityComponents.insert({ComponentIDMap[id], std::make_shared<ComponentList<T>>()});
        }
        //std::cout << __func__ << "<" << id << ">" << "() -> " << ComponentIDMap[id] << "\n";
        /// is that necessary?
        return ComponentIDMap[id];
    }

    Components &getComponents(EntityID id);
    

    template <typename T>
    ComponentID getComponentID()
    {
        ComponentType componentIndex = ComponentType(typeid(T).name());
        //std::cout << __func__ << "<" << typeid(T).name() << ">() -> ComponentIDMap[" << componentIndex << "]" << ComponentIDMap[componentIndex] << "\n";
        return ComponentIDMap[componentIndex];
    }

    template <typename T>
    void addComponent(EntityID id, T c)
    {
        //std::cout << __func__ << "(" << id << ", " << &c << ")\n";
        ComponentID cId = registerComponent<T>();
        entityComponentMap[id].set(cId, true);
        addEntityComponent(id, cId, c);
    }

    template <typename T>
    T &getComponent(EntityID id)
    {
        //std::cout << __func__ << "<" << typeid(id).name() << ">(" << id << ") -> ...\n";
        return std::static_pointer_cast<ComponentList<T>>(entityComponents[ComponentIDMap[ComponentType(typeid(T).name())]])->get(entityComponentIndices[id][ComponentIDMap[ComponentType(typeid(T).name())]]);
    }

    template <typename T>
    void addEntityComponent(EntityID id, ComponentID cId, T &c)
    {
        //std::cout << __func__ << "(" << id << ", " << cId << ", "  << &c << ")\n";
        if ( entityComponents.find(cId) == entityComponents.end() )
        {
            entityComponents.insert({cId, std::make_shared<ComponentList<T>>(c)});
            exit(3);
        }
        uint32_t idx = std::static_pointer_cast<ComponentList<T>>(entityComponents[cId])->add(c);
        entityComponentIndices[id][cId] = idx;
    }
    
    SystemID newSystem(std::string name);
    

    SystemID addSystem(Components components, std::string name);
    

    bool updateSystemComponents(SystemID system, Components components);
    

    std::vector<EntityID> getSystemEntities(SystemID sid)
    {
        return systemEntities[sid];
    }

    void updateSystem(SystemID system);
    

    void showAll();
    
};

class System
{
    public:
    const EntityID id;

    System(EntityID id) : id(id) {}

    virtual void update(float deltaTimeS) = 0;

    virtual void init(bool initial) {
        if ( initial )
        {
            (void)EntityManager::getInstance().updateSystemComponents(id, components);
        }
        else
        {
            EntityManager::getInstance().updateSystem(id);
        }
        EntityManager::getInstance().showAll();
    }
protected:
    Components components;
};


#endif // _ECS_H_
