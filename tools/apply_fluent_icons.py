import argparse
import io
import re
import shutil
from pathlib import Path

from PIL import Image
from resvg_py import svg_to_bytes


ROOT = Path(__file__).resolve().parents[1]
VENDORED = ROOT / "third_party" / "fluentui-system-icons"
SVG_DIR = VENDORED / "svg"
OUTPUT_DIR = ROOT / "MusicPlayer2" / "res" / "fluent"
RC_PATH = ROOT / "MusicPlayer2" / "MusicPlayer2.rc"

ALIASES = {
    "play_in_playlist": "Playlist Add", "play_in_folder": "Folder Open",
    "play_as_next": "Next", "play_pause": "Play Circle",
    "previous": "Previous", "上一曲": "Previous",
    "next": "Next", "下一曲": "Next",
    "pause": "Pause", "暂停": "Pause",
    "play": "Play", "播放": "Play",
    "stop": "Stop", "停止": "Stop",
    "volume0": "Speaker Mute", "volume1": "Speaker 0",
    "volume2": "Speaker 1", "volume3": "Speaker 2",
    "setting": "Settings", "close": "Dismiss", "delete": "Delete",
    "addtag": "Tag Multiple", "replacetag": "Tag", "deletetag": "Tag Dismiss",
    "add": "Add", "playlist_float": "Window New", "playlist_dock": "Window",
    "playlist": "Text Bullet List", "播放列表": "Text Bullet List",
    "media_lib": "Library", "menu": "Navigation", "heart": "Heart",
    "favourite": "Heart", "skn": "Color", "skin": "Color",
    "曲目信息": "Info", "info": "Info", "karaoke": "Mic",
    "顺序播放": "Arrow Repeat All", "列表循环": "Arrow Repeat All",
    "单曲循环": "Arrow Repeat 1", "单曲播放": "Music Note 2",
    "随机播放": "Arrow Shuffle", "play_random": "Arrow Shuffle",
    "play_shuffle": "Arrow Shuffle", "maximize": "Maximize",
    "minimize": "Subtract", "restore": "Square Multiple",
    "folder_explore": "Folder Search", "explorer_folder": "Folder Open",
    "new_folder": "Folder Add", "文件夹": "Folder", "folder": "Folder",
    "full_screen1": "Full Screen Minimize", "full_screen": "Full Screen Maximize",
    "mini_restore": "Window Multiple", "mini": "Window New",
    "find": "Search", "查找": "Search", "replace": "Arrow Sync",
    "fast_forward": "Fast Forward", "ff_new": "Fast Forward", "快进": "Fast Forward",
    "rew_new": "Rewind", "快退": "Rewind", "rewind": "Rewind",
    "equalizer": "Settings", "eq": "Settings", "save_as": "Save Copy", "save": "Save",
    "lock": "Lock Closed", "double_line": "Text Column Two", "edit": "Edit",
    "album_cover": "Image", "album": "Album", "artist": "Person",
    "year": "Calendar", "genre": "Music Note 2", "music": "Music Note 2",
    "lyric_delay": "Clock", "lyric_forward": "Arrow Right", "internal_lyric": "Text",
    "lyric": "Text", "recent_songs": "History", "file_relate": "Document Link",
    "online": "Globe", "convert": "Arrow Sync", "download1": "Arrow Download",
    "download": "Arrow Download", "help": "Question Circle", "statistics": "Data Histogram",
    "pin": "Pin", "exit": "Sign Out", "dark_mode": "Weather Moon",
    "light_mode": "Weather Sunny", "ok": "Checkmark Circle", "rename": "Rename",
    "tag": "Tag", "locate": "Location", "star_select": "Star", "star": "Star",
    "expand": "Chevron Down", "sort": "Arrow Sort", "display_mode": "Grid",
    "unlink": "Link Dismiss", "link": "Link", "speed_up": "Arrow Trending",
    "slow_down": "Arrow Trending Down", "shortcut": "Open", "switch": "Arrow Swap",
    "copy": "Copy", "bitrate": "Gauge", "reverb": "Sound Wave Circle",
    "hot_key": "Keyboard", "now_playing": "Play Circle", "refresh": "Arrow Clockwise",
    "background": "Image", "tree_branch_collapsed": "Chevron Right",
    "tree_branch_expanded": "Chevron Down", "drop_down": "Chevron Down",
    "list_preview": "Text Bullet List", "arrow_left": "Arrow Left",
    "checkbox": "Checkmark Square", "more": "More Horizontal", "up": "Chevron Up",
    "fix": "Wrench",
}

TARGET_TOKENS = tuple(ALIASES)
SKIP = ("MusicPlayer2", "default_cover", "file_icon", "file_playlist_icon")
SOURCE_FALLBACKS = {
    "Playlist Add": "Text Bullet List",
    "Tag Multiple": "Tag",
    "Tag Dismiss": "Tag",
    "Save Copy": "Save",
    "Folder Search": "Folder",
    "Document Link": "Document",
    "Arrow Trending": "Arrow Up Right",
    "Arrow Trending Down": "Arrow Down Right",
    "Sound Wave Circle": "Speaker 2",
    "Open": "Open Folder",
}


def choose_icon(stem: str) -> str:
    key = stem.lower()
    for token, name in sorted(ALIASES.items(), key=lambda item: -len(item[0])):
        if token.lower() in key:
            return name
    raise KeyError(stem)


def source_svg(source_root: Path, name: str, filled: bool) -> Path:
    style = "filled" if filled else "regular"
    folder = source_root / "assets" / name / "SVG"
    preferred = (
        folder / f"ic_fluent_{name.lower().replace(' ', '_')}_24_{style}.svg",
        folder / f"ic_fluent_{name.lower().replace(' ', '_')}_20_{style}.svg",
    )
    for path in preferred:
        if path.exists():
            return path
    matches = sorted(folder.glob(f"*_{style}.svg"))
    if not matches:
        fallback = SOURCE_FALLBACKS.get(name, "More Horizontal")
        if fallback == name:
            raise FileNotFoundError(f"Fluent SVG not found: {name} ({style})")
        return source_svg(source_root, fallback, filled)
    return matches[0]


def vendored_svg(name: str, filled: bool) -> Path:
    style = "filled" if filled else "regular"
    return SVG_DIR / f"{name.lower().replace(' ', '_')}_24_{style}.svg"


def import_sources(source_root: Path, requirements: set[tuple[str, bool]]) -> None:
    SVG_DIR.mkdir(parents=True, exist_ok=True)
    for name, filled in sorted(requirements):
        shutil.copy2(source_svg(source_root, name, filled), vendored_svg(name, filled))
    shutil.copy2(source_root / "LICENSE", VENDORED / "LICENSE")


def recolor_svg(path: Path, color: str) -> str:
    data = path.read_text(encoding="utf-8")
    data = data.replace('fill="#212121"', f'fill="{color}"')
    data = data.replace('fill="currentColor"', f'fill="{color}"')
    return data


def generate_ico(svg: Path, target: Path, color: str) -> None:
    data = recolor_svg(svg, color)
    images = []
    for size in (16, 20, 24, 32, 48, 64):
        png = svg_to_bytes(data, width=size, height=size)
        images.append(Image.open(io.BytesIO(png)).convert("RGBA"))
    target.parent.mkdir(parents=True, exist_ok=True)
    images[-1].save(target, format="ICO", sizes=[(s, s) for s in (16, 20, 24, 32, 48, 64)])


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--import-source", type=Path)
    args = parser.parse_args()

    rc = RC_PATH.read_text(encoding="utf-16")
    pattern = re.compile(r'^(\s*IDI_\S+\s+ICON\s+")res\\\\([^\"]+\.ico)(".*)$', re.M)
    resources = []
    for match in pattern.finditer(rc):
        filename = match.group(2)
        stem = Path(filename).stem
        resource_id_match = re.search(r"IDI_\S+", match.group(1))
        if not resource_id_match:
            continue
        resource_id = resource_id_match.group(0)
        if any(item.lower() in filename.lower() for item in SKIP):
            continue
        semantic_key = f"{stem} {resource_id}"
        name = choose_icon(semantic_key)
        filled = "heart" in semantic_key.lower() or "select" in semantic_key.lower()
        resources.append((resource_id, filename, name, filled))

    if not resources:
        raise RuntimeError("No target ICON resources matched MusicPlayer2.rc")

    requirements = {(name, filled) for _, _, name, filled in resources}
    if args.import_source:
        import_sources(args.import_source.resolve(), requirements)

    replacements = {}
    for resource_id, filename, name, filled in resources:
        svg = vendored_svg(name, filled)
        if not svg.exists():
            raise FileNotFoundError(f"Vendored source missing: {svg}")
        output_name = resource_id.lower() + ".ico"
        target = OUTPUT_DIR / output_name
        is_dark_icon = resource_id.lower().endswith("_d") or "dark" in Path(filename).stem.lower()
        generate_ico(svg, target, "#202020" if is_dark_icon else "#F5F5F5")
        replacements[filename] = output_name

    def replace_resource(match: re.Match) -> str:
        filename = match.group(2)
        output_name = replacements.get(filename)
        if not output_name:
            return match.group(0)
        return f'{match.group(1)}res\\\\fluent\\\\{output_name}{match.group(3)}'

    RC_PATH.write_text(pattern.sub(replace_resource, rc), encoding="utf-16")
    print(f"Generated {len(replacements)} Fluent ICO resources in {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
