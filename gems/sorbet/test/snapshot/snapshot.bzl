
def snapshot_tests(paths, test_prefix):

    tests = []

    for path in paths:
        test_name = 'test_{}/{}'.format(test_prefix, path)

        lock_env = "@installed_gems//gems/sorbet/test/snapshot/{}/src:Gemfile.lock-env".format(path)

        data = [
            "//main:sorbet",
            "//gems/sorbet:sorbet",
            "{}/src".format(path),
        ]

        native.sh_test(
            name = test_name,
            srcs = [ "test_one_bazel.sh" ],
            data = data,
            deps = [
                ":logging",
                "@ruby_2_4_3//:ruby",
                lock_env,
            ],
            args = [
                "$(location @ruby_2_4_3//:ruby)",
                "$(location {})".format(lock_env),
                path,
            ],
        )

        tests.append(test_name)

    return tests
