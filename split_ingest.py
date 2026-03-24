from gitingest import ingest
import os
import re
import fnmatch
import traceback

def split_content_by_files(content):
    separator = "=" * 48
    parts = content.split(separator)
    file_blocks = []
    i = 1
    while i < len(parts) - 1:
        header_part = parts[i]
        body_part = parts[i+1]
        if "FILE:" in header_part.upper():
            file_blocks.append({
                "header": header_part, "body": body_part, "separator": separator
            })
            i += 2
        else:
            i += 1
    return file_blocks

def recover_binary_files(file_blocks):
    recovered_count = 0
    filename_pattern = re.compile(r'(?:FILE|File|file):\s*(.*)', re.IGNORECASE)

    for block in file_blocks:
        if "[Binary file]" in block["body"]:
            match = filename_pattern.search(block["header"])
            if match:
                filename = match.group(1).strip().replace('\\', '/')
                if os.path.exists(filename) and os.path.isfile(filename):
                    try:
                        with open(filename, 'r', encoding='utf-8', errors='replace') as f:
                            content = f.read()
                        block["body"] = "\n" + content + "\n"
                        recovered_count += 1
                    except Exception as e:
                        print(f"      ⚠️ Failed to recover {filename}: {e}")
    if recovered_count > 0:
        print(f"      🚑 Successfully forced-read {recovered_count} files marked as binary.")
    return file_blocks

def truncate_large_arrays(file_blocks, target_patterns):
    if not target_patterns: return file_blocks
    print(f"   ✂️  Applying truncation to patterns: {', '.join(target_patterns)}")
    
    processed_blocks = []
    truncated_count = 0
    array_pattern = re.compile(r'(=[\s\r\n]*\{)([\s\S]*?)(\}[\s\r\n]*;)', re.DOTALL)
    filename_pattern = re.compile(r'(?:FILE|File|file):\s*(.*)', re.IGNORECASE)

    for block in file_blocks:
        header = block["header"]
        body = block["body"]
        match = filename_pattern.search(header)
        filename = match.group(1).strip().replace('\\', '/') if match else "unknown"
        
        normalized_patterns = [p.replace('\\', '/') for p in target_patterns]
        should_truncate = any(fnmatch.fnmatch(filename, p) for p in normalized_patterns)

        if should_truncate:
            original_len = len(body)
            def replacement(m):
                content = m.group(2)
                if len(content) > 200:
                    lines = content.count('\n')
                    return f"{m.group(1)}\n    /* ... (DATA TRUNCATED: ~{len(content)} chars / {lines} lines) ... */\n{m.group(3)}"
                return m.group(0)
            new_body = array_pattern.sub(replacement, body)
            if len(new_body) < original_len:
                block["body"] = new_body
                truncated_count += 1
                print(f"      🔹 Truncated: {filename}")
        processed_blocks.append(block)
    
    if truncated_count > 0:
        print(f"      ✅ Truncated massive arrays in {truncated_count} files.")
    return processed_blocks

def save_blocks(prefix, suffix, summary, tree, file_blocks, max_size_kb):
    common_header = f"{summary}\n\n{tree}\n\n"
    current_chunk_idx = 1
    current_content = ""
    max_bytes = max_size_kb * 1024 if max_size_kb > 0 else float('inf')
    text_blocks = [f"{b['separator']}{b['header']}{b['separator']}{b['body']}" for b in file_blocks]

    print(f"   ℹ️  Total {len(text_blocks)} files packing into {'unlimited' if max_size_kb==0 else str(max_size_kb) + 'KB'} units.")

    for block in text_blocks:
        if max_size_kb > 0 and current_content and (len(common_header) + len(current_content) + len(block) > max_bytes):
            filename = f"{prefix}-{suffix}-{current_chunk_idx}.txt"
            with open(filename, "w", encoding="utf-8") as f:
                f.write(common_header + current_content)
            print(f"      📄 {filename} created ({len(current_content)//1024} KB)")
            current_chunk_idx += 1
            current_content = ""
        current_content += block

    if current_content:
        filename = f"{prefix}-{suffix}-{current_chunk_idx}.txt" if max_size_kb > 0 else f"{prefix}-{suffix}.txt"
        with open(filename, "w", encoding="utf-8") as f:
            f.write(common_header + current_content)
        print(f"      📄 {filename} created ({len(current_content)//1024} KB)")

def main():
    print("==========================================")
    print("   VS C++ Project: Smart Split Ingest v4.2")
    print("==========================================")
    
    prefix = input("👉 Enter project name (e.g., cs250_hw): ").strip()
    if not prefix: return

    size_input = input("👉 Source split size (KB) (0 for no split, default 500): ").strip()
    if size_input == "": size_input = "500"
    max_size_kb = int(size_input) if size_input.isdigit() else 0

    # ⭐ 추출 옵션 추가 (y/n)
    setting_input = input("👉 Extract Settings (sln, vcxproj, etc.)? (y/n, default: y): ").strip().lower()
    extract_settings = setting_input != 'n'  # n을 치지 않으면 기본적으로 추출

    tp_input = input("👉 Extract Third-party (SDL2, etc.)? (y/n, default: y): ").strip().lower()
    extract_tp = tp_input != 'n'

    trunc_input = input("👉 Truncate large arrays (e.g. data tables)? (y/n): ").strip().lower()
    trunc_patterns = []
    if trunc_input == 'y':
        print("   Target files? (wildcards allowed, e.g. *data.cpp, *table.h)")
        p_input = input("   (Leave EMPTY to SKIP truncation): ").strip()
        if p_input:
            trunc_patterns = [p.strip() for p in p_input.split(",")]

    common_exclude = [
        ".git/*", "*/.git/*", 
        ".vs/*", "*/.vs/*", 
        "out/*", "*/out/*", 
        "build/*", "*/build/*", 
        "x64/*", "*/x64/*", 
        "Debug/*", "*/Debug/*", 
        "Release/*", "*/Release/*",
        "*.obj", "*.pdb", "*.exe", "*.dll", "*.lib", "*.ilk", "*.tlog", 
        "*.lastbuildstate", "*.idb", "*.ipch", "*.suo", "*.user",
        "*.png", "*.jpg", "*.bmp"
    ]

    print(f"\n🚀 Analyzing '{prefix}'...")

    try:
        # 1️⃣ Structure (구조는 용량이 작고 전체 파악에 중요하므로 항상 추출)
        s, t, _ = ingest(".", exclude_patterns=common_exclude)
        with open(f"{prefix}-structure.txt", "w", encoding="utf-8") as f:
            f.write(s + "\n" + t)
        print(f"1️⃣  [Structure] Done")

        # 2️⃣ Settings (조건부 추출)
        if extract_settings:
            setting_patterns = [
                "*/visual_studio/*", "**/*visual_studio*", "*.sln", "*/sln/*", 
                "*.vcxproj", "*.filters", "README.md", ".gitignore"
            ]
            s, t, c = ingest(".", include_patterns=setting_patterns, exclude_patterns=common_exclude)
            with open(f"{prefix}-setting.txt", "w", encoding="utf-8") as f:
                f.write(f"{s}\n\n{t}\n\n{c}")
            print(f"2️⃣  [Settings] Done")
        else:
            print(f"2️⃣  [Settings] Skipped")

        # 3️⃣ Third Party (조건부 추출)
        if extract_tp:
            third_party_patterns = ["*third_party*", "*/third_party/*", "**/third_party/**"]
            s, t, c = ingest(".", include_patterns=third_party_patterns, exclude_patterns=common_exclude)
            
            tp_blocks = split_content_by_files(c)
            tp_blocks = recover_binary_files(tp_blocks)
            if trunc_patterns:
                tp_blocks = truncate_large_arrays(tp_blocks, trunc_patterns)
            
            if tp_blocks:
                save_blocks(prefix, "third_party", s, t, tp_blocks, max_size_kb)
            print(f"3️⃣  [Third Party] Done")
        else:
            print(f"3️⃣  [Third Party] Skipped")

        # 4️⃣ Source (항상 추출하되, third_party와 visual_studio는 확실히 제외!)
        source_patterns = ["*.cpp", "*.h", "*.c", "*.hpp", "*.inl"]
        source_exclude = common_exclude + [
            "*third_party*", "*/third_party/*", "**/third_party/**",
            "*visual_studio*", "*/visual_studio/*", "**/visual_studio/**"
        ]
        s, t, c = ingest(".", include_patterns=source_patterns, exclude_patterns=source_exclude)
        
        src_blocks = split_content_by_files(c)
        src_blocks = recover_binary_files(src_blocks)
        if trunc_patterns:
            src_blocks = truncate_large_arrays(src_blocks, trunc_patterns)
            
        if src_blocks:
            save_blocks(prefix, "source", s, t, src_blocks, max_size_kb)
        print(f"4️⃣  [Source] Done")

    except Exception as e:
        print(f"❌ Error: {e}")
        traceback.print_exc()

    print("\n✨ Process completed!")

if __name__ == "__main__":
    main()