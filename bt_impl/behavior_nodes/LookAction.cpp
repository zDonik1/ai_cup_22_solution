/**************************************************************************
 *
 *   @author Doniyorbek Tokhirov <tokhirovdoniyor@gmail.com>
 *   @date 11/07/2022
 *
 *************************************************************************/

#include "LookAction.h"

#include "model/Vec2.hpp"

using namespace std;
using namespace BT;
using namespace model;

constexpr auto COS_THRESHOLD = 0.99;

LookAction::LookAction(const EnemyMap &enemies,
                       const Unit &unit,
                       UnitOrder &order,
                       const string &name,
                       const NodeConfiguration &config)
    : StatefulActionNode{name, config}, m_enemies{enemies}, m_unit{unit}, m_order{order}
{}

PortsList LookAction::providedPorts()
{
    return {InputPort<Vec2>("vector"), InputPort<int>("id")};
}

NodeStatus LookAction::onStart()
{
    auto vector = getInput<Vec2>("vector");
    auto id = getInput<int>("id");
    if (!vector && !id) {
        cout << "Couldn't find port: vector or id" << endl;
        return NodeStatus::FAILURE;
    }

    m_order.targetDirection = id ? m_enemies.at(id.value()).get().position - m_unit.position
                                 : vector.value();
    if (dotProduct(m_order.targetDirection.normalize(), m_unit.direction) > COS_THRESHOLD) {
        return NodeStatus::SUCCESS;
    } else {
        return NodeStatus::RUNNING;
    }
}

NodeStatus LookAction::onRunning()
{
    return onStart();
}

void LookAction::onHalted() {}
