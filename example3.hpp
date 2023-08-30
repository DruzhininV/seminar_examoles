#include <concepts>
#include <memory>
#include <vector>
#include <string>
#include <ranges>
#include <fmt/core.h>
#include <iostream>



namespace app {

template <typename T>
concept streamable = requires(T value, std::ostream out) {
    { out << value } -> std::convertible_to<std::ostream&>;
};

void dump(streamable auto const& value) {
std::cout << value << "\n";
}

template<typename T>
concept app_state = std::copyable<T> and requires (T value) {
    dump(value);
};


class AppStateItem {
public:
    ~AppStateItem() = default;
    AppStateItem(AppStateItem&&) noexcept = default;
    AppStateItem& operator=(AppStateItem&&) noexcept = default;
    AppStateItem(AppStateItem const& obj) : model_{ obj.model_->copy() } {}
    AppStateItem& operator=(AppStateItem const& obj) {
        auto tmp{ obj };
        model_ = std::move(tmp.model_);
        return *this;
    }

    template<typename T> AppStateItem(T state)
            : model_{ std::make_unique<AppStateModel<T>>(std::move(state)) } {}

    friend void dump(AppStateItem const& val) {
        val.model_->dump_();
    }

private:

    struct AppStateConcept;
    using Concept = std::unique_ptr<AppStateConcept>;

    struct AppStateConcept {
        virtual ~AppStateConcept() = default;
        virtual Concept copy() const = 0;
        virtual void dump_() = 0;
    };

    template<app_state T>
    struct AppStateModel final: AppStateConcept {
        explicit AppStateModel(T state) : data_{ std::move(state) } {}
        ~AppStateModel() final = default;

        [[nodiscard]] Concept copy() const final {
            return std::make_unique<AppStateModel>(*this);
        }

        void dump_() final {
            dump(data_);
        }

        T data_;
    };

    Concept model_;
};

static_assert(std::copyable<AppStateItem>);
static_assert(app_state<AppStateItem>);

using AppState = std::vector<AppStateItem>;

void dump(AppState const& container) {
    std::cout << "Container begin\n";
    std::for_each(container.begin(), container.end(), [&](app_state auto const& state) {
        dump(state);
    });
    std::cout << "Container end\n";
}


} //namespace app;

using namespace app;


struct MyType{};

void dump(MyType value) {
    std::cout << "MyType\n";
}

int main() {
    std::cout << fmt::format("{:=^80}\n", "<examples>");

    AppState appState{};

    appState.emplace_back(0);
    appState.emplace_back(std::numbers::egamma);
    appState.emplace_back(std::string{"string"});
    appState.emplace_back(MyType{});
    appState[0] = 15;

    dump(appState);

    std::cout << fmt::format("{:=^80}\n", "</examples>");
}