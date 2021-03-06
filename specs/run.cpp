#include <specs/fakes/fake_reporter.h>
#include <specs/util/argv_helper.h>
#include <specs/specs.h>

using namespace fakes;
using namespace specs::util;

namespace bd = bandit::detail;

go_bandit([]() {
  describe("run", [&]() {
    std::unique_ptr<bd::spec_registry> specs;
    std::unique_ptr<argv_helper> argv;
    fake_reporter* reporter;
    std::unique_ptr<bd::controller_t> controller;

    auto call_run = [&]() -> int {
      bd::options opt(argv->argc(), argv->argv());
      return run(opt, *specs, *controller);
    };

    before_each([&]() {
      specs.reset(new bd::spec_registry());
      reporter = new fake_reporter();
      controller.reset(new bd::controller_t());
      controller->set_reporter(reporter);
      argv.reset(new argv_helper({}));
    });

    it("pushes the global context on the context stack", [&]() {
      call_run();
      AssertThat(controller->get_contexts(), Is().OfLength(1));
    });

    describe("a successful test run", [&]() {
      int number_of_specs_called;

      before_each([&]() {
        number_of_specs_called = 0;
        specs->push_back([&]() { number_of_specs_called++; });
      });

      it("calls the context", [&]() {
        call_run();
        AssertThat(number_of_specs_called, Equals(1));
      });

      it("tells reporter a test run is about to start", [&]() {
        call_run();
        AssertThat(reporter->call_log(), Has().Exactly(1).EqualTo("test_run_starting"));
      });

      it("tells reporter a test run has completed", [&]() {
        call_run();
        AssertThat(reporter->call_log(), Has().Exactly(1).EqualTo("test_run_complete"));
      });

      it("returns 0 as no specs failed", [&]() {
        AssertThat(call_run(), Equals(0));
      });
    });

    describe("a failing test run", [&]() {
      before_each([&]() {
        reporter->set_test_run_status(false);
      });

      it("returns a non-zero error code", [&]() {
        AssertThat(call_run(), IsGreaterThan(0));
      });
    });
  });
});
