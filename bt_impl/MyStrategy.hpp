#pragma once

#include "DebugInterface.hpp"
#include "model/Constants.hpp"
#include "model/Game.hpp"
#include "model/Order.hpp"

#include <behaviortree_cpp_v3/bt_factory.h>
#include <behaviortree_cpp_v3/loggers/bt_cout_logger.h>

class MyStrategy
{
    template<typename T>
    using Ref = std::reference_wrapper<T>;

public:
    MyStrategy(const model::Constants &constants);
    model::Order getOrder(model::Game &&game, DebugInterface *debugInterface);
    void debugUpdate(int displayedTick, DebugInterface &debugInterface);
    void finish();

private:
    void registerNodes();
    void initTree();

private:
    BT::Blackboard::Ptr m_blackboard;
    BT::BehaviorTreeFactory m_factory;
    BT::Tree m_tree;
    std::unique_ptr<BT::StdCoutLogger> m_logger;

    model::Constants m_constants;
    model::Game m_game;
    model::Unit m_dummyUnit;
    model::Unit &m_unit = m_dummyUnit;
    std::unordered_map<int, Ref<model::Unit>> m_enemies;

    model::UnitOrder m_order;
    std::unordered_map<int, model::UnitOrder> m_orders;
};
