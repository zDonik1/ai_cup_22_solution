/**************************************************************************
 *
 *   @author Doniyorbek Tokhirov <tokhirovdoniyor@gmail.com>
 *   @date 14/07/2022
 *
 *************************************************************************/

#include "GoToTarget.h"

using namespace std;
using namespace BT;
using namespace model;

constexpr auto SHOOT_RANGE_RATIO = 2.0 / 3.0;

GoToTarget::GoToTarget(const Constants &constants,
                       const EnemyMap &enemies,
                       const Unit &unit,
                       UnitOrder &order,
                       const string &name,
                       const NodeConfiguration &config)
    : StatefulActionNode{name, config},
      m_constants{constants}, m_enemies{enemies}, m_unit{unit}, m_order{order}
{}

PortsList GoToTarget::providedPorts()
{
    return {InputPort<int>("id")};
}

NodeStatus GoToTarget::onStart()
{
    if (!m_unit.weapon)
        return NodeStatus::FAILURE;

    auto id = getInput<int>("id");
    if (!id) {
        cout << "Couldn't find port: id" << endl;
        return NodeStatus::FAILURE;
    }

    const auto unitWeapon = m_constants.weapons.at(m_unit.weapon.value());
    const auto weaponRange = unitWeapon.projectileLifeTime * unitWeapon.projectileSpeed;
    const auto shootRange = weaponRange * SHOOT_RANGE_RATIO;
    const auto toEnemyVector = m_enemies.at(id.value()).get().position - m_unit.position;
    if (toEnemyVector.sqrLength() <= shootRange * shootRange) {
        return NodeStatus::SUCCESS;
    } else {
        m_order.targetVelocity = normalizeVelocity(toEnemyVector, m_constants.maxUnitForwardSpeed);
        return NodeStatus::RUNNING;
    }
}

NodeStatus GoToTarget::onRunning()
{
    return onStart();
}

void GoToTarget::onHalted() {}
