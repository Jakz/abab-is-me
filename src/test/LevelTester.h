#include "game/Types.h"

namespace baba::test
{
  struct MoveSequence
  {
    std::vector<D> moves;
  public:
    MoveSequence(const std::string& string);

    auto begin() const { return moves.begin(); }
    auto end() const { return moves.end(); }
  };
  
  
  class LevelTester
  {
  protected:
    World* _world;

  public:
    LevelTester();
    void test(const std::string levelName, const MoveSequence& moves);
  };
}