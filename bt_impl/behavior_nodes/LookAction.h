/**************************************************************************
 *
 *   @author Doniyorbek Tokhirov <tokhirovdoniyor@gmail.com>
 *   @date 11/07/2022
 *
 *************************************************************************/

#pragma once

#include <behaviortree_cpp_v3/action_node.h>

#include "model/Game.hpp"
#include "model/UnitOrder.hpp"

class LookAction : public BT::StatefulActionNode
{
public:
    LookAction(const EnemyMap &enemies,
               const model::Unit &unit,
               model::UnitOrder &order,
               const std::string &name,
               const BT::NodeConfiguration &config);

    static BT::PortsList providedPorts();

    virtual BT::NodeStatus onStart() override;
    virtual BT::NodeStatus onRunning() override;
    virtual void onHalted() override;

private:
    const EnemyMap &m_enemies;
    const model::Unit &m_unit;
    model::UnitOrder &m_order;
};
