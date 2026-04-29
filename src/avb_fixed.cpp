// avb_fixed.cpp
// AVB/vbmeta parser and patcher — output matches `avbtool info_image`
// Adapted to UtilBase pattern for the utils project.

#include "../include/main.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <vector>

using u8  = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

// ─── minimal SHA-1 for public key fingerprint ────────────────────────────────

static void avb_sha1(const u8* data, size_t len, u8 out[20]) {
    u32 h0=0x67452301,h1=0xEFCDAB89,h2=0x98BADCFE,h3=0x10325476,h4=0xC3D2E1F0;
    auto rol=[](u32 v,int n)->u32{return (v<<n)|(v>>(32-n));};
    std::vector<u8> msg(data, data+len);
    msg.push_back(0x80);
    while (msg.size()%64!=56) msg.push_back(0);
    u64 bits=(u64)len*8;
    for(int i=7;i>=0;i--) msg.push_back((bits>>(i*8))&0xFF);
    for(size_t i=0;i<msg.size();i+=64){
        u32 w[80];
        for(int j=0;j<16;j++)
            w[j]=(u32(msg[i+j*4])<<24)|(u32(msg[i+j*4+1])<<16)|(u32(msg[i+j*4+2])<<8)|msg[i+j*4+3];
        for(int j=16;j<80;j++) w[j]=rol(w[j-3]^w[j-8]^w[j-14]^w[j-16],1);
        u32 a=h0,b=h1,c=h2,d=h3,e=h4;
        for(int j=0;j<80;j++){
            u32 f,k;
            if(j<20){f=(b&c)|(~b&d);k=0x5A827999;}
            else if(j<40){f=b^c^d;k=0x6ED9EBA1;}
            else if(j<60){f=(b&c)|(b&d)|(c&d);k=0x8F1BBCDC;}
            else{f=b^c^d;k=0xCA62C1D6;}
            u32 t=rol(a,5)+f+e+k+w[j]; e=d;d=c;c=rol(b,30);b=a;a=t;
        }
        h0+=a;h1+=b;h2+=c;h3+=d;h4+=e;
    }
    u32 hh[5]={h0,h1,h2,h3,h4};
    for(int i=0;i<5;i++){
        out[i*4+0]=(hh[i]>>24)&0xFF; out[i*4+1]=(hh[i]>>16)&0xFF;
        out[i*4+2]=(hh[i]>>8)&0xFF;  out[i*4+3]= hh[i]&0xFF;
    }
}

// ─── Buf helper ──────────────────────────────────────────────────────────────

struct AvbBuf {
    const u8* d{};
    size_t sz{};

    bool slice(size_t off, size_t len, AvbBuf& out) const {
        if (off > sz || len > sz - off) return false;
        out = {d + off, len};
        return true;
    }
    bool be32(size_t off, u32& v) const {
        if (off + 4 > sz) return false;
        v = (u32(d[off])<<24)|(u32(d[off+1])<<16)|(u32(d[off+2])<<8)|d[off+3];
        return true;
    }
    bool be64(size_t off, u64& v) const {
        u32 hi, lo;
        if (!be32(off,hi)||!be32(off+4,lo)) return false;
        v = (u64(hi)<<32)|lo;
        return true;
    }
    std::string str(size_t off, size_t len) const {
        if (off > sz || len > sz - off) return {};
        return {reinterpret_cast<const char*>(d+off), len};
    }
};

// ─── formatting helpers ───────────────────────────────────────────────────────

static void avb_print_hex(const u8* p, size_t n) {
    for (size_t i = 0; i < n; ++i)
        printf("%02x", p[i]);
}

// ─── AVB Header ──────────────────────────────────────────────────────────────

struct AvbHeader {
    u64 auth{}, aux{};
    u32 algo{};
    u64 hash_off{}, hash_sz{};
    u64 sig_off{},  sig_sz{};
    u64 pk_off{},   pk_sz{};
    u64 pkmeta_off{}, pkmeta_sz{};
    u64 desc_off{}, desc_sz{};
    u64 rollback{};
    u32 flags{};
    u32 rollback_loc{};
    std::string release;
};

static const char* avb_algo_name(u32 a) {
    switch (a) {
        case 0: return "NONE";
        case 1: return "SHA256_RSA2048";
        case 2: return "SHA256_RSA4096";
        case 3: return "SHA256_RSA8192";
        case 4: return "SHA512_RSA2048";
        case 5: return "SHA512_RSA4096";
        case 6: return "SHA512_RSA8192";
        default: return "UNKNOWN";
    }
}

static bool avb_parse_header(const AvbBuf& f, AvbHeader& h) {
    if (f.sz < 256 || std::memcmp(f.d, "AVB0", 4) != 0) return false;
    if (!f.be64(0x0C,h.auth)||!f.be64(0x14,h.aux)) return false;
    if (!f.be32(0x1C,h.algo)) return false;
    if (!f.be64(0x20,h.hash_off)||!f.be64(0x28,h.hash_sz)) return false;
    if (!f.be64(0x30,h.sig_off) ||!f.be64(0x38,h.sig_sz))  return false;
    if (!f.be64(0x40,h.pk_off)  ||!f.be64(0x48,h.pk_sz))   return false;
    if (!f.be64(0x50,h.pkmeta_off)||!f.be64(0x58,h.pkmeta_sz)) return false;
    if (!f.be64(0x60,h.desc_off)||!f.be64(0x68,h.desc_sz)) return false;
    if (!f.be64(0x70,h.rollback)) return false;
    if (!f.be32(0x78,h.flags)||!f.be32(0x7C,h.rollback_loc)) return false;
    h.release = f.str(0x80, 48);
    auto z = h.release.find('\0');
    if (z != std::string::npos) h.release.resize(z);
    return true;
}

// ─── Descriptor parsers ───────────────────────────────────────────────────────

// TAG 2 — AvbHashDescriptor
static void avb_parse_hash(const AvbBuf& b) {
    u64 image_size;
    u32 name_len, salt_len, dig_len, flags;
    if (!b.be64( 0, image_size)) return;
    std::string halg(reinterpret_cast<const char*>(b.d + 8),
                     strnlen(reinterpret_cast<const char*>(b.d + 8), 32));
    if (!b.be32(40, name_len))  return;
    if (!b.be32(44, salt_len))  return;
    if (!b.be32(48, dig_len))   return;
    if (!b.be32(52, flags))     return;
    size_t off = 116;
    if (off + name_len + salt_len + dig_len > b.sz) return;
    std::string part = b.str(off, name_len); off += name_len;
    const u8* salt   = b.d + off;            off += salt_len;
    const u8* digest = b.d + off;
    printf("    Hash descriptor:\n");
    printf("      Image Size:            %llu bytes\n", (unsigned long long)image_size);
    printf("      Hash Algorithm:        %s\n",      halg.c_str());
    printf("      Partition Name:        %s\n",      part.c_str());
    printf("      Salt:                  "); avb_print_hex(salt, salt_len);   printf("\n");
    printf("      Digest:                "); avb_print_hex(digest, dig_len); printf("\n");
    printf("      Flags:                 %u\n",      flags);
}

// TAG 1 — AvbHashtreeDescriptor
static void avb_parse_hashtree(const AvbBuf& b) {
    u32 ver, dbs, hbs, fec_roots, name_len, salt_len, root_len, flags;
    u64 image_size, tree_off, tree_sz, fec_off, fec_sz;
    if (!b.be32(  0, ver))       return;
    if (!b.be64(  4, image_size))return;
    if (!b.be64( 12, tree_off))  return;
    if (!b.be64( 20, tree_sz))   return;
    if (!b.be32( 28, dbs))       return;
    if (!b.be32( 32, hbs))       return;
    if (!b.be32( 36, fec_roots)) return;
    if (!b.be64( 40, fec_off))   return;
    if (!b.be64( 48, fec_sz))    return;
    std::string halg(reinterpret_cast<const char*>(b.d + 56),
                     strnlen(reinterpret_cast<const char*>(b.d + 56), 32));
    if (!b.be32( 88, name_len))  return;
    if (!b.be32( 92, salt_len))  return;
    if (!b.be32( 96, root_len))  return;
    if (!b.be32(100, flags))     return;
    size_t off = 164;
    if (off + name_len + salt_len + root_len > b.sz) return;
    std::string part = b.str(off, name_len); off += name_len;
    const u8* salt   = b.d + off;            off += salt_len;
    const u8* root   = b.d + off;
    printf("    Hashtree descriptor:\n");
    printf("      Version of dm-verity:  %u\n",        ver);
    printf("      Image Size:            %llu bytes\n", (unsigned long long)image_size);
    printf("      Tree Offset:           %llu\n",       (unsigned long long)tree_off);
    printf("      Tree Size:             %llu bytes\n", (unsigned long long)tree_sz);
    printf("      Data Block Size:       %u bytes\n",   dbs);
    printf("      Hash Block Size:       %u bytes\n",   hbs);
    printf("      FEC num roots:         %u\n",         fec_roots);
    printf("      FEC offset:            %llu\n",       (unsigned long long)fec_off);
    printf("      FEC size:              %llu bytes\n", (unsigned long long)fec_sz);
    printf("      Hash Algorithm:        %s\n",         halg.c_str());
    printf("      Partition Name:        %s\n",         part.c_str());
    printf("      Salt:                  "); avb_print_hex(salt, salt_len); printf("\n");
    printf("      Root Digest:           "); avb_print_hex(root, root_len); printf("\n");
    printf("      Flags:                 %u\n",         flags);
}

// TAG 4 — AvbChainPartitionDescriptor
static void avb_parse_chain(const AvbBuf& b) {
    u32 rollback_loc, name_len, pk_len, flags;
    if (!b.be32( 0, rollback_loc)) return;
    if (!b.be32( 4, name_len))     return;
    if (!b.be32( 8, pk_len))       return;
    if (!b.be32(12, flags))        return;
    size_t off = 76;
    if (off + name_len + pk_len > b.sz) return;
    std::string part = b.str(off, name_len); off += name_len;
    const u8* pk = b.d + off;
    u8 fp[20];
    avb_sha1(pk, pk_len, fp);
    printf("    Chain Partition descriptor:\n");
    printf("      Partition Name:          %s\n",  part.c_str());
    printf("      Rollback Index Location: %u\n",  rollback_loc);
    printf("      Public key (sha1):       "); avb_print_hex(fp, 20); printf("\n");
    printf("      Flags:                   %u\n",  flags);
}

// TAG 0 — AvbPropertyDescriptor
static void avb_parse_property(const AvbBuf& b) {
    u64 key_len, val_len;
    if (!b.be64(0, key_len) || !b.be64(8, val_len)) return;
    size_t off = 16;
    if (off + key_len + 1 + val_len > b.sz) return;
    std::string key = b.str(off, key_len); off += key_len + 1;
    std::string val = b.str(off, val_len);
    printf("    Prop: %s -> '%s'\n", key.c_str(), val.c_str());
}

// TAG 3 — AvbKernelCmdlineDescriptor
static void avb_parse_cmdline(const AvbBuf& b) {
    u32 flags, cmdline_len;
    if (!b.be32(0, flags) || !b.be32(4, cmdline_len)) return;
    if (8 + cmdline_len > b.sz) return;
    std::string cmdline = b.str(8, cmdline_len);
    printf("    Kernel Cmdline descriptor:\n");
    printf("      Flags:                 %u\n", flags);
    printf("      Kernel Cmdline:        '%s'\n", cmdline.c_str());
}

// ─── Flags ───────────────────────────────────────────────────────────────────

static constexpr u32 AVB_FLAG_HASHTREE_DISABLED     = 1u << 0;
static constexpr u32 AVB_FLAG_VERIFICATION_DISABLED = 1u << 1;

static const char* avb_flag_state(u32 flags) {
    bool ht = flags & AVB_FLAG_HASHTREE_DISABLED;
    bool vv = flags & AVB_FLAG_VERIFICATION_DISABLED;
    if (!ht && !vv) return "FULLY ENABLED (locked)";
    if ( ht && !vv) return "PARTIAL — hashtree disabled, signatures still checked";
    if (!ht &&  vv) return "PARTIAL — verification disabled, dm-verity still active";
    return "FULLY DISABLED (both flags set)";
}

static void avb_put_be32(u8* p, size_t off, u32 v) {
    p[off+0] = (v >> 24) & 0xFF;
    p[off+1] = (v >> 16) & 0xFF;
    p[off+2] = (v >>  8) & 0xFF;
    p[off+3] =  v        & 0xFF;
}

// ─── Print info ───────────────────────────────────────────────────────────────

static void avb_print_info(const AvbHeader& h, const AvbBuf& aux) {
    u8 pk_fp[20] = {};
    bool has_pk = false;
    if (h.pk_sz > 0 && h.pk_off + h.pk_sz <= aux.sz) {
        avb_sha1(aux.d + static_cast<size_t>(h.pk_off),
                 static_cast<size_t>(h.pk_sz), pk_fp);
        has_pk = true;
    }
    printf("Minimum libavb version:   1.0\n");
    printf("Header Block:             256 bytes\n");
    printf("Authentication Block:     %llu bytes\n", (unsigned long long)h.auth);
    printf("Auxiliary Block:          %llu bytes\n", (unsigned long long)h.aux);
    if (has_pk) {
        printf("Public key (sha1):        "); avb_print_hex(pk_fp, 20); printf("\n");
    }
    printf("Algorithm:                %s\n", avb_algo_name(h.algo));
    printf("Rollback Index:           %llu\n", (unsigned long long)h.rollback);
    printf("Flags:                    %u\n",   h.flags);
    printf("Rollback Index Location:  %u\n",   h.rollback_loc);
    printf("Release String:           '%s'\n", h.release.c_str());
    printf("Descriptors:\n");

    size_t pos = static_cast<size_t>(h.desc_off);
    size_t end = pos + static_cast<size_t>(h.desc_sz);

    while (pos + 16 <= aux.sz && pos < end) {
        u64 tag, n;
        if (!aux.be64(pos, tag) || !aux.be64(pos + 8, n)) break;
        if (n == 0 || pos + 16 + n > aux.sz) break;
        AvbBuf body;
        if (!aux.slice(pos + 16, static_cast<size_t>(n), body)) break;
        switch (tag) {
            case 0: avb_parse_property(body); break;
            case 1: avb_parse_hashtree(body); break;
            case 2: avb_parse_hash(body);     break;
            case 3: avb_parse_cmdline(body);  break;
            case 4: avb_parse_chain(body);    break;
            default:
                printf("    Unknown descriptor tag=%llu size=%llu\n",
                       (unsigned long long)tag, (unsigned long long)n);
                break;
        }
        pos += 16 + static_cast<size_t>(n);
    }

    printf("\nFlags:\n");
    printf("  HASHTREE_DISABLED:      %s\n",
           (h.flags & 1) ? "true  [WARNING: dm-verity runtime check is OFF]" : "false");
    printf("  VERIFICATION_DISABLED:  %s\n",
           (h.flags & 2) ? "true  [WARNING: all AVB signature checks are OFF]" : "false");
    printf("\nOverall verification: %s\n", avb_flag_state(h.flags));
}

// ─── Patchers ────────────────────────────────────────────────────────────────

static ProcessResult avb_patch_disable(std::vector<u8>& data, u32 old_flags,
                                        const std::string& path) {
    bool ht = old_flags & AVB_FLAG_HASHTREE_DISABLED;
    bool vv = old_flags & AVB_FLAG_VERIFICATION_DISABLED;

    if (ht && vv) {
        printf("\n[PATCH] Nothing to do — protection is already fully disabled.\n");
        return ProcessResult::ok;
    }

    u32 new_flags = old_flags;
    int method = 0;

    if (!ht && !vv) {
        new_flags = AVB_FLAG_HASHTREE_DISABLED | AVB_FLAG_VERIFICATION_DISABLED;
        method = 3;
        printf("\n[PATCH] Method 3 — disable-verity + disable-verification\n");
        printf("        Device is fully locked; setting both flags.\n");
        printf("        Equivalent: avbctl --force disable-verity\n");
        printf("                    avbctl --force disable-verification\n");
    } else if (!ht && vv) {
        new_flags = old_flags | AVB_FLAG_HASHTREE_DISABLED;
        method = 1;
        printf("\n[PATCH] Method 1 — disable-verity\n");
        printf("        VERIFICATION_DISABLED already set; adding HASHTREE_DISABLED.\n");
        printf("        Equivalent: avbctl --force disable-verity\n");
    } else {
        new_flags = old_flags | AVB_FLAG_VERIFICATION_DISABLED;
        method = 2;
        printf("\n[PATCH] Method 2 — disable-verification\n");
        printf("        HASHTREE_DISABLED already set; adding VERIFICATION_DISABLED.\n");
        printf("        Equivalent: avbctl --force disable-verification\n");
    }

    avb_put_be32(data.data(), 0x78, new_flags);

    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        fprintf(stderr, "ERROR: cannot open '%s' for writing\n", path.c_str());
        return ProcessResult::write_error;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!ofs) return ProcessResult::write_error;

    printf("\n        Before : flags=0x%02x  (%s)\n", old_flags, avb_flag_state(old_flags));
    printf("        After  : flags=0x%02x  (%s)\n",  new_flags, avb_flag_state(new_flags));
    printf("        Method : %d\n",                  method);
    printf("        File   : %s\n\n",                path.c_str());
    printf("[PATCH] Done. The AVB RSA signature is now INVALID — expected for a patched file.\n\n");
    printf("How to flash:\n");
    printf("  Option A — flash THIS patched file (no extra flags needed):\n");
    printf("    fastboot flash vbmeta %s\n\n", path.c_str());
    printf("  Option B — flash the STOCK file with fastboot flags:\n");
    if (method == 1)
        printf("    fastboot --disable-verity flash vbmeta <stock_vbmeta.img>\n");
    else if (method == 2)
        printf("    fastboot --disable-verification flash vbmeta <stock_vbmeta.img>\n");
    else
        printf("    fastboot --disable-verity --disable-verification flash vbmeta <stock_vbmeta.img>\n");

    return ProcessResult::ok;
}

static ProcessResult avb_patch_restore(std::vector<u8>& data, u32 old_flags,
                                        const std::string& path) {
    if (!(old_flags & (AVB_FLAG_HASHTREE_DISABLED | AVB_FLAG_VERIFICATION_DISABLED))) {
        printf("\n[RESTORE] Nothing to do — protection flags are already clear.\n");
        return ProcessResult::ok;
    }

    u32 new_flags = old_flags & ~(AVB_FLAG_HASHTREE_DISABLED | AVB_FLAG_VERIFICATION_DISABLED);
    avb_put_be32(data.data(), 0x78, new_flags);

    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        fprintf(stderr, "ERROR: cannot open '%s' for writing\n", path.c_str());
        return ProcessResult::write_error;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!ofs) return ProcessResult::write_error;

    printf("\n[RESTORE] Before : flags=0x%02x  %s\n", old_flags, avb_flag_state(old_flags));
    printf("[RESTORE] After  : flags=0x%02x  %s\n",  new_flags, avb_flag_state(new_flags));
    printf("[RESTORE] File   : %s\n\n", path.c_str());
    printf("[RESTORE] Done. Re-flash with the original signed vbmeta to fully re-lock.\n");
    return ProcessResult::ok;
}

// ─── UtilBase interface ───────────────────────────────────────────────────────

void AvbFixed::show_help() {
    fprintf(stderr,
        "\navb_fixed\n\n"
        "Usage:\n\n"
        "    avb_fixed <vbmeta.img> [--disable_verified | --restore]\n\n"
        "    <vbmeta.img>          Path to the vbmeta image file\n"
        "    (no flags)            Print full AVB info (default)\n"
        "    --disable_verified    Disable all AVB/dm-verity protection\n"
        "    --restore             Re-enable protection flags\n\n"
        "Patch methods used by --disable_verified:\n"
        "    Method 1 — set HASHTREE_DISABLED only      (verification already off)\n"
        "    Method 2 — set VERIFICATION_DISABLED only  (hashtree already off)\n"
        "    Method 3 — set both flags                  (device fully locked)\n\n"
        "WARNING: patching invalidates the AVB RSA signature.\n"
        "         The device must have an unlocked bootloader to boot a patched vbmeta.\n\n"
    );
}

ParseResult AvbFixed::parse_cmd_line(int argc, char* argv[]) {
    // argc/argv already stripped of binary name and subcommand by main()
    // argv[0] = vbmeta.img path, optional flags follow
    if (argc < 1) {
        show_help();
        return ParseResult::not_enough;
    }

    do_disable = false;
    do_restore = false;
    image_path.clear();

    for (int i = 0; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--disable_verified") {
            do_disable = true;
        } else if (arg == "--restore") {
            do_restore = true;
        } else if (arg.starts_with("-")) {
            fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
            show_help();
            return ParseResult::wrong_option;
        } else {
            if (!image_path.empty()) {
                fprintf(stderr, "Too many positional arguments\n\n");
                show_help();
                return ParseResult::wrong_option;
            }
            image_path = argv[i];
        }
    }

    if (image_path.empty()) {
        show_help();
        return ParseResult::not_enough;
    }
    if (do_disable && do_restore) {
        fprintf(stderr, "Cannot use --disable_verified and --restore together\n\n");
        show_help();
        return ParseResult::wrong_option;
    }
    return ParseResult::ok;
}

ProcessResult AvbFixed::process() {
    // Load file
    std::ifstream fs(image_path, std::ios::binary);
    if (!fs) {
        fprintf(stderr, "Cannot open: %s\n", image_path.c_str());
        return ProcessResult::open_error;
    }
    std::vector<u8> data((std::istreambuf_iterator<char>(fs)), {});
    if (data.empty()) {
        fprintf(stderr, "Empty file\n");
        return ProcessResult::read_error;
    }

    AvbBuf file{data.data(), data.size()};
    AvbHeader h{};
    if (!avb_parse_header(file, h)) {
        fprintf(stderr, "Invalid vbmeta (bad magic or too small)\n");
        return ProcessResult::read_error;
    }

    size_t auth_off = 256;
    size_t aux_off  = auth_off + static_cast<size_t>(h.auth);
    if (aux_off > file.sz) {
        fprintf(stderr, "Broken layout: aux block out of file\n");
        return ProcessResult::read_error;
    }
    AvbBuf aux{file.d + aux_off,
               std::min<size_t>(static_cast<size_t>(h.aux), file.sz - aux_off)};

    // Always print info first
    avb_print_info(h, aux);

    if (do_disable) return avb_patch_disable(data, h.flags, image_path);
    if (do_restore) return avb_patch_restore(data, h.flags, image_path);

    return ProcessResult::ok;
}
