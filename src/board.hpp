#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace panel {
constexpr size_t MAX_CARDS = 10;

struct Card {
  const char *id;
  const char *title;
  const char *unit;
  uint8_t decimals;
  float value = 0.0f;
};

class Cards {
  friend class Board;

public:
  Cards(std::initializer_list<Card> cards);
  bool update(const char *id, float value);

private:
  Card cards[MAX_CARDS];
  size_t count;
};

class Board {
public:
  Board(const char *title, std::initializer_list<Card> cards)
      : title(title), cards(cards) {}
  bool updateCard(const char *id, float value);
  size_t serializeLayout(char *buffer, size_t size) const;
  size_t serializeValues(char *buffer, size_t size) const;

private:
  const char *title;
  Cards cards;
};
} // namespace panel
