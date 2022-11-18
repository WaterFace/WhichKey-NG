#include "HUD.h"

namespace HUD {
  enum : std::uint32_t { kPrompt, kName, kLock };
  struct SendHUDMessageThunk {
    static void thunk(
        RE::UIMessageQueue* a_this,
        const RE::BSFixedString& a_menuName,
        RE::UI_MESSAGE_TYPE a_type,
        RE::IUIMessageData* a_data) {
      const auto data = a_data ? static_cast<RE::HUDData*>(a_data) : nullptr;
      const auto crossHairRef = data ? data->crossHairRef.get() : RE::TESObjectREFRPtr();

      if (data && crossHairRef && crossHairRef->IsLocked()) {
        // If the thing we're looking at is locked...
        if (auto* lock = crossHairRef->GetLock(); lock) {
          if (auto key = lock->key; key) {
            // if it has a key...
            if (RE::PlayerCharacter::GetSingleton()->GetItemCount(key) > 0) {
              // if the player has a copy of that key...
              std::string text{data->text};
              auto splitText = stl::string::split(text, "\n");
              if (splitText.size() > kLock) {  // Then this side of the door is actually locked
                splitText[kLock] = std::format(diamondMarkerFmt, key->GetFullName());
                text = stl::string::join(splitText, "\n");
                data->text = text;
              }
              // Otherwise, it's the other side that's locked.
            }
          }
        }
      }
      func(a_this, a_menuName, a_type, a_data);
    }

    static inline REL::Relocation<decltype(thunk)> func;

    static inline constexpr auto diamondMarkerFmt{
        "<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>{}"sv};
  };

  void Install() {
    REL::Relocation<std::uintptr_t> target{
        RELOCATION_ID(39535, 40621),
        static_cast<ptrdiff_t>(RELOCATION_OFFSET(0x289, 0x280, 0x0))};  // TODO: get the VR offset
    stl::write_thunk_call<SendHUDMessageThunk>(target.address());
  }
}  // namespace HUD