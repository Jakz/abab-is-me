#include "LevelTester.h"

#include "game/World.h"
#include "game/Level.h"

using namespace baba::test;

baba::test::MoveSequence::MoveSequence(const std::string& string)
{
  moves.reserve(string.size());

  size_t count = 0;

  for (size_t i = 0; i < string.size(); ++i)
  {
    char c = string[i];

    switch (c)
    {
      case 'r':
        for (size_t i = 0; i < std::max(count, 1ull); ++i)
          moves.push_back(D::RIGHT);
        count = 0;
        break;
      case 'l':
        for (size_t i = 0; i < std::max(count, 1ull); ++i)
          moves.push_back(D::LEFT);
        count = 0;
        break;
      case 'u':
        for (size_t i = 0; i < std::max(count, 1ull); ++i)
          moves.push_back(D::UP);
        count = 0;
        break;
      case 'd':
        for (size_t i = 0; i < std::max(count, 1ull); ++i)
          moves.push_back(D::DOWN);
        count = 0;
        break;
      case 'w':
        for (size_t i = 0; i < std::max(count, 1ull); ++i)
          moves.push_back(D::NONE);
        count = 0;
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        count *= 10;
        count += c - '0';
        break;
    }
  }
}

baba::test::LevelTester::LevelTester()
{
  _world = new baba::World();
}

void baba::test::LevelTester::test(const std::string levelName, const MoveSequence& moves, bool victory)
{
  _world->loadLevel(levelName, false);

  for (const auto& move : moves)
    _world->level()->movement(move);

  bool success = (victory && _world->level()->isVictory()) || (!victory && _world->level()->isDefeat());

  printf("%s\n", fmt::format("[TEST] Level '{}' victory: {}", levelName, success ? "yes" : "no").c_str());
}