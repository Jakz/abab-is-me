#include "LevelTester.h"

#include "game/World.h"
#include "game/Level.h"

using namespace baba::test;

baba::test::MoveSequence::MoveSequence(const std::string& string)
{
  moves.reserve(string.size());

  for (const auto& c : string)
  {
    switch (c)
    {
      case 'r': moves.push_back(D::RIGHT); break;
      case 'l': moves.push_back(D::LEFT); break;
      case 'u': moves.push_back(D::UP); break;
      case 'd': moves.push_back(D::DOWN); break;
      case 'w': moves.push_back(D::NONE); break;
      default: break;
    }
  }
}

baba::test::LevelTester::LevelTester()
{
  _world = new baba::World();
}

void baba::test::LevelTester::test(const std::string levelName, const MoveSequence& moves)
{
  _world->loadLevel(levelName, false);

  for (const auto& move : moves)
    _world->level()->movement(move);

  printf("%s\n", fmt::format("[TEST] Level '{}' victory: {}", levelName, _world->level()->isVictory() ? "yes" : "no").c_str());
}