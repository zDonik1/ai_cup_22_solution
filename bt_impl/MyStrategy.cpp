#include "MyStrategy.hpp"

#include <exception>
#include <iostream>
#include <memory>

#include <behaviortree_cpp_v3/basic_types.h>

#include "behavior_nodes/LookAction.h"
#include "behavior_nodes/GoToTarget.h"

using namespace std;
using namespace BT;
using namespace model;

constexpr auto DEBUG = false;
constexpr auto BEHAVIOR_FILE = "main_behavior.xml";

namespace BT {

template<>
inline Vec2 convertFromString(StringView str)
{
    auto parts = splitString(str, ',');
    if (parts.size() != 2) {
        throw RuntimeError("invalid input in behavior tree");
    } else {
        Vec2 vector;
        vector.x = convertFromString<float>(parts[0]);
        vector.y = convertFromString<float>(parts[1]);
        return vector;
    }
}

} // namespace BT


MyStrategy::MyStrategy(const Constants &constants)
    : m_blackboard{Blackboard::create()}, m_constants{constants}
{
    registerNodes();
    initTree();
}

Order MyStrategy::getOrder(Game &&game, DebugInterface *debugInterface)
{
    m_enemies.clear();
    m_order = {};

    // state initialization
    m_game = move(game);
    for (auto &unit : m_game.units) {
        if (unit.playerId == m_game.myId) {
            m_unit = unit;
        } else {
            m_enemies.emplace(unit.id, ref(unit));
        }
    }

    // ticking
    if (m_tree.tickRoot() == NodeStatus::FAILURE) {
        return Order{unordered_map<int, UnitOrder>()};
    } else {
        m_orders[m_unit.id] = m_order;
        return Order{m_orders};
    }
}

void MyStrategy::debugUpdate(int displayedTick, DebugInterface &debugInterface) {}

void MyStrategy::finish() {}

void MyStrategy::registerNodes()
{
    const auto lookBuilder = [this](const string &name, const NodeConfiguration &config) {
        return make_unique<LookAction>(m_enemies, m_unit, m_order, name, config);
    };

    const auto goToTargetBuilder = [this](const string &name, const NodeConfiguration &config) {
        return make_unique<GoToTarget>(m_constants, m_enemies, m_unit, m_order, name, config);
    };

    // --- ports list

    PortsList vectorPort = {InputPort<Vec2>("vector")};
    PortsList idOutPort = {OutputPort<int>("id")};
    PortsList idInPort = {InputPort<int>("id")};


    // ---- action nodes

    m_factory.registerBuilder<LookAction>("Look", lookBuilder);
    m_factory.registerBuilder<GoToTarget>("GoToTarget", goToTargetBuilder);

    m_factory.registerSimpleAction(
        "Move",
        [this](TreeNode &self) {
            auto vector = self.getInput<Vec2>("vector");
            if (!vector) {
                cout << "Couldn't find port: vector" << endl;
                return NodeStatus::FAILURE;
            }

            m_order.targetVelocity = normalizeVelocity(vector.value(),
                                                       m_constants.maxUnitForwardSpeed);
            return NodeStatus::SUCCESS;
        },
        vectorPort);

    m_factory.registerSimpleAction("Dodge", [this](TreeNode &) {
        Vec2 result;
        bool hasIntersect = false;
        for (const Projectile &projectile : m_game.projectiles) {
            if (projectile.shooterPlayerId == m_game.myId)
                continue;

            const auto [intersect, normal] = rayCircleIntersectNormalVector(projectile.position,
                                                                            projectile.velocity,
                                                                            m_unit.position,
                                                                            m_constants.unitRadius);
            if (intersect) {
                result += normal;
                hasIntersect = true;
            }
        }

        if (hasIntersect) {
            m_order.targetVelocity = normalizeVelocity(result, m_constants.maxUnitForwardSpeed);
        }
        return hasIntersect ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    });

    m_factory.registerSimpleAction("AvoidZone", [this](TreeNode &) {
        constexpr auto PADDING = 0.5; // units
        const auto unitToZoneVec = m_game.zone.currentCenter - m_unit.position;
        if (m_game.zone.currentRadius - unitToZoneVec.length() < m_constants.unitRadius + PADDING) {
            m_order.targetVelocity = normalizeVelocity(unitToZoneVec, m_constants.maxUnitForwardSpeed);
            return NodeStatus::SUCCESS;
        } else {
            return NodeStatus::FAILURE;
        }
    });

    m_factory.registerSimpleAction(
        "GetClosestTarget",
        [this](TreeNode &self) {
            const auto it = min_element(
                cbegin(m_enemies), cend(m_enemies), [this](const auto &left, const auto &right) {
                    return (m_unit.position - left.second.get().position).sqrLength()
                           < (m_unit.position - right.second.get().position).sqrLength();
                });

            if (it == cend(m_enemies)) {
                return NodeStatus::FAILURE;
            } else {
                self.setOutput("id", it->first);
                return NodeStatus::SUCCESS;
            }
        },
        idOutPort);

    m_factory.registerSimpleAction(
        "Shoot",
        [this](TreeNode &self) {
            if (!m_unit.weapon || m_unit.ammo.at(m_unit.weapon.value()) == 0)
                return NodeStatus::FAILURE;

            m_order.action = make_shared<ActionOrder::Aim>(true);
            return NodeStatus::SUCCESS;
        },
        idInPort);

    m_factory.registerSimpleAction("GoCenter", [this](TreeNode &self) {
        const auto direction = m_game.zone.currentCenter - m_unit.position;
        m_order.targetVelocity = normalizeVelocity(direction, m_constants.maxUnitForwardSpeed);
        m_order.targetDirection = direction;
        return NodeStatus::SUCCESS;
    });

    // ---- condition nodes

    // ---- decorator nodes
}

void MyStrategy::initTree()
{
    try {
        m_tree = m_factory.createTreeFromFile(string{BEHAVIORS_PATH} + BEHAVIOR_FILE,
                                              m_blackboard);
        if (DEBUG) {
            m_logger = make_unique<StdCoutLogger>(m_tree);
        }
    } catch (const runtime_error &e) {
        cout << e.what() << endl;
    }
}
