//================================
//https://godbolt.org/z/cWK997x16
//================================


#include <concepts>
#include <memory>
#include <vector>
#include <string>
#include <ranges>
#include <fmt/core.h>
#include <iostream>
#include <cassert>
#include <future>



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
    template<app_state T> explicit AppStateItem(T state)
            : model_{ std::make_shared<AppStateModel<T> const>(std::move(state)) } {}

    template<typename T, typename DumpStrategy>
    requires std::invocable<DumpStrategy, T>
    AppStateItem(T state, DumpStrategy strategy)
            : model_{ std::make_shared<CustomizableModel<T, DumpStrategy> const>(std::move(state), std::move(strategy))}
    {}

    friend void dump(AppStateItem const& val) {
        val.model_->dump_();
    }

private:
    struct AppStateConcept;
    using Concept = std::shared_ptr<AppStateConcept const>;

    struct AppStateConcept {
        virtual ~AppStateConcept() = default;
        virtual Concept copy() const = 0;
        virtual void dump_() const = 0;
    };

    template<app_state T>
    struct AppStateModel final: AppStateConcept {
        explicit AppStateModel(T state) : data_{ std::move(state) } {}
        ~AppStateModel() final = default;

        [[nodiscard]] Concept copy() const final {
            return std::make_unique<AppStateModel>(*this);
        }

        void dump_() const final {
            dump(data_);
        }

        T data_;
    };

    template<app_state T, typename DumpStrategy>
    requires std::invocable<DumpStrategy, T>
    struct CustomizableModel final : AppStateConcept {
        CustomizableModel(T state, DumpStrategy strategy)
                : data_{ std::move(state) }, dumpStrategy_{ std::move(strategy) } {}
        ~CustomizableModel() final = default;

        [[nodiscard]] Concept copy() const final {
            return std::make_unique<CustomizableModel>(*this);
        }

        void dump_() const final {
            dumpStrategy_(data_);
        }

        T data_;
        DumpStrategy dumpStrategy_;
    };

    Concept model_;
};

static_assert(std::copyable<AppStateItem>);
static_assert(app_state<AppStateItem>);

using AppState = std::vector<AppStateItem>;
using History = std::vector<AppState>;

void dump(AppState const& state) {
    std::cout << fmt::format("{:=^80}\n", "begin");
    for(auto const& item : state) {
        dump(item);
    }
    std::cout << fmt::format("{:=^80}\n", "end");
}

History createHistory(size_t capacity) {
    History history{capacity};
    history.push_back({});
    return history;
}

void commit(History& history) {
    assert(!history.empty());
    history.push_back(history.back());
}

void undo(History& history) {
    assert(!history.empty());
    history.pop_back();
}

AppState& current(History& history) {
    assert(!history.empty());
    return history.back();
}


} //namespace app;

using namespace app;


struct MyType{};

void dump(MyType value) {
    std::cout << "MyType\n";
}

int main() {
    std::cout << fmt::format("{:=^80}\n", "<examples>");

    auto dbDumper{[](app_state auto const& state){
        std::cout << fmt::format("Dump into DataBase {}\n", state);
    }};

    auto history{createHistory(10)};

    current(history).emplace_back(1);
    current(history).emplace_back(std::numbers::pi);
    current(history).emplace_back("string");

    std::cout << fmt::format("{:=^80}\n", "State 1");
    dump(current(history));

    commit(history);

    current(history)[2] = AppStateItem("new string", dbDumper);
    current(history).emplace_back(current(history));
    current(history).emplace_back(MyType{});

    auto asyncDump{std::async([state = current(history)]{
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << fmt::format("{:=^80}\n", "Async dump");
        dump(state);
    })};

    std::cout << fmt::format("{:=^80}\n", "State 2");
    dump(current(history));

    std::cout << fmt::format("{:=^80}\n", "Undo");
    undo(history);
    dump(current(history));


    std::cout << fmt::format("{:=^80}\n", "</examples>");
}