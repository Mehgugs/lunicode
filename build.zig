const std = @import("std");

// Type helpers
const String     = []const u8;
const StringPair = [2]String;
const SourceList = std.ArrayList(String);
const FlagList   = std.ArrayList(String);
fn SliceOf(comptime t: type) type {
    return []const t;
}

//builder
const Builder = std.build.Builder;
const Step    = std.build.Step;
const InstallDir = std.build.InstallDir;
const Artifact = *std.build.LibExeObjStep;

pub fn build(b: *Builder) !void {
    const art = b.addSharedLibrary("lunicode", null, .unversioned);
    var flags = FlagList.init(b.allocator);

    //Target & build mode
    const target = b.standardTargetOptions(.{.default_target = std.zig.CrossTarget.fromTarget(b.host.target)});
    const mode = std.builtin.Mode.ReleaseSmall; // NB. Hardcoded to ReleaseSmall to prevent errors on windows; investigate.

    const luapathopt = b.option(String, "lua", "Path to your lua installation.");
    if (luapathopt) |luapath| {
        art.addIncludePath(b.pathJoin(&.{luapath, "include"}));
        art.addLibraryPath(b.pathJoin(&.{luapath, "lib"}));
        art.strip = true;
        art.bundle_compiler_rt = false;
    } else {
        std.log.err("No lua path provided!", .{});
        std.os.exit(1);
    }

    art.setBuildMode(mode);
    art.setTarget(target);
    art.linkLibC();
    art.linkSystemLibrary("lua5.4.4");
    art.addIncludePath("utf8proc");

    try flags.append("-DUTF8PROC_STATIC");

    if(target.os_tag == std.Target.Os.Tag.windows) {
        try flags.append("-DLUA_USE_WINDOWS");
    }
    else if ((target.os_tag == std.Target.Os.Tag.linux) or (target.os_tag == std.Target.Os.Tag.macos)) {
        try flags.append("-DLUA_USE_POSIX");
        try flags.append("-fPIC");
    }

    art.addCSourceFiles(&.{"src/lunicode.c", "utf8proc/utf8proc.c"}, flags.toOwnedSlice());

    art.install();

}