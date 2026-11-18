#include "board.hpp"

#include <cstring>
#include <mutex>

#include <ArduinoJson.h>

namespace panel {
namespace {
std::mutex s_mutex;
}

Cards::Cards(std::initializer_list<Card> items) : cards{}, count(0) {
  for (const Card &card : items) {
    if (count >= MAX_CARDS) {
      break;
    }
    cards[count] = card;
    cards[count].value = 0.0f;
    ++count;
  }
}

bool Cards::update(const char *id, float value) {
  if (id == nullptr) {
    return false;
  }

  std::lock_guard<std::mutex> lock(s_mutex);
  bool found = false;
  for (size_t i = 0; i < count; ++i) {
    if (cards[i].id != nullptr && std::strcmp(cards[i].id, id) == 0) {
      cards[i].value = value;
      found = true;
      break;
    }
  }
  return found;
}

bool Board::updateCard(const char *id, float value) {
  return cards.update(id, value);
}

size_t Board::serializeLayout(char *buffer, size_t size) const {
  if (buffer == nullptr || size == 0 || title == nullptr) {
    return 0;
  }

  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["title"] = title;
  JsonArray arr = root["metrics"].to<JsonArray>();
  for (size_t i = 0; i < cards.count; ++i) {
    const Card &card = cards.cards[i];
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = card.id;
    obj["title"] = card.title;
    obj["unit"] = card.unit;
    obj["decimals"] = card.decimals;
  }

  return serializeJson(doc, buffer, size);
}

size_t Board::serializeValues(char *buffer, size_t size) const {
  if (buffer == nullptr || size == 0) {
    return 0;
  }

  float values[MAX_CARDS];
  const char *ids[MAX_CARDS];
  size_t n = 0;

  {
    std::lock_guard<std::mutex> lock(s_mutex);
    n = cards.count;
    for (size_t i = 0; i < n; ++i) {
      ids[i] = cards.cards[i].id;
      values[i] = cards.cards[i].value;
    }
  }

  JsonDocument doc;
  JsonArray arr = doc["metrics"].to<JsonArray>();
  for (size_t i = 0; i < n; ++i) {
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = ids[i];
    obj["value"] = values[i];
  }

  return serializeJson(doc, buffer, size);
}
} // namespace panel
