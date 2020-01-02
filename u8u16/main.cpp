#include <iostream>
#include <memory>
#include <chrono>
#include <random>
#include "u8u16.hpp"
#include "basic_duration.h"

#include "gsl/gsl"

int main()
{
    // UTF-16 string length
    constexpr const size_t u16Length{ 100000000u }; // 100,000 code points
    //constexpr const size_t u16Length{ 10000000u }; // 10,000 code points

    // UTF-16 character to be used
    //const std::wstring testU16(u16Length, gsl::narrow_cast<wchar_t>(0x007E)); // TILDE (1 Byte in UTF-8)
    //const std::wstring testU16(u16Length, gsl::narrow_cast<wchar_t>(0x00F6)); // LATIN SMALL LETTER O WITH DIAERESIS (2 Bytes in UTF-8)
    const std::wstring testU16(u16Length, gsl::narrow_cast<wchar_t>(0x20AC)); // // EURO SIGN (3 Bytes in UTF-8)

    NTSTATUS(WINAPI * p_RtlUTF8ToUnicodeN)
    (
        _Out_ PWSTR UnicodeStringDestination,
        _In_ ULONG UnicodeStringMaxByteCount,
        _Out_opt_ PULONG UnicodeStringActualByteCount,
        _In_ PCCH UTF8StringSource,
        _In_ ULONG UTF8StringByteCount) = nullptr;

    NTSTATUS(WINAPI * p_RtlUnicodeToUTF8N)
    (
        _Out_ PCHAR UTF8StringDestination,
        _In_ ULONG UTF8StringMaxByteCount,
        _Out_opt_ PULONG UTF8StringActualByteCount,
        _In_ PCWSTR UnicodeStringSource,
        _In_ ULONG UnicodeStringWCharCount) = nullptr;

    HMODULE ntdll = LoadLibraryA("ntdll.dll");
    if (ntdll != nullptr)
    {
        [[gsl::suppress(26490)]] // suppress NO_REINTERPRET_CAST
        {
            p_RtlUTF8ToUnicodeN = reinterpret_cast<NTSTATUS(WINAPI*)(PWSTR, ULONG, PULONG, PCCH, ULONG)>(GetProcAddress(ntdll, "RtlUTF8ToUnicodeN"));
            p_RtlUnicodeToUTF8N = reinterpret_cast<NTSTATUS(WINAPI*)(PCHAR, ULONG, PULONG, PCWSTR, ULONG)>(GetProcAddress(ntdll, "RtlUnicodeToUTF8N"));
        }
        if (!p_RtlUTF8ToUnicodeN || !p_RtlUnicodeToUTF8N)
        {
            FreeLibrary(ntdll);
            return 1;
        }
    }

    std::default_random_engine generator{  gsl::narrow_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()) };
    std::uniform_int_distribution<size_t> distribution{};
    NTSTATUS status{};
    HRESULT hRes{};
    int length{};
    ULONG written{};
    double duration{};
    char randElem8{};
    wchar_t randElem16{};

    std::cout << "*** UTF-16 To UTF-8 ***\n"
              << std::endl;

    GetBasicDuration();
    std::unique_ptr<char[]> u8Buffer{ std::make_unique<char[]>(u16Length * 3) };
    length = WideCharToMultiByte(65001, 0, testU16.c_str(), gsl::narrow_cast<int>(u16Length), u8Buffer.get(), gsl::narrow_cast<int>(u16Length) * 3, nullptr, nullptr);
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, gsl::narrow_cast<size_t>(length)};
    [[gsl::suppress(26446)]] randElem8 = u8Buffer[distribution(generator)];
    u8Buffer.reset();
    std::cout << "WideCharToMultiByte" << "\n random character " << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(randElem8))
              << "\n length " << length << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    GetBasicDuration();
    std::unique_ptr<char[]> u8Buffer2{ std::make_unique<char[]>(u16Length * 3) };
    status = p_RtlUnicodeToUTF8N(u8Buffer2.get(), gsl::narrow_cast<ULONG>(u16Length) * 3, &written, testU16.c_str(), gsl::narrow_cast<ULONG>(testU16.length() * 2));
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, gsl::narrow_cast<size_t>(written)};
    [[gsl::suppress(26446)]] randElem8 = u8Buffer2[distribution(generator)];
    u8Buffer2.reset();
    std::cout << "RtlUnicodeToUTF8N" << "\n random character " << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(randElem8))
              << "\n NTSTATUS " << status << "\n length " << written << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    GetBasicDuration();
    std::string u8Str{};
    hRes = U16ToU8(testU16, u8Str);
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, u8Str.length()};
    randElem8 = u8Str.at(distribution(generator));
    std::cout << "U16ToU8" << "\n random character " << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(randElem8))
              << "\n HRESULT " << hRes << "\n length " << u8Str.length() << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    std::cout << "\n*** UTF-8 To UTF-16 ***\n"
              << std::endl;

    GetBasicDuration();
    std::unique_ptr<wchar_t[]> u16Buffer{ std::make_unique<wchar_t[]>(u8Str.length()) };
    length = MultiByteToWideChar(65001, 0, u8Str.c_str(), gsl::narrow_cast<int>(u8Str.length()), u16Buffer.get(), gsl::narrow_cast<int>(u8Str.length()));
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, gsl::narrow_cast<size_t>(length)};
    [[gsl::suppress(26446)]] randElem16 = u16Buffer[distribution(generator)];
    u16Buffer.reset();
    std::cout << "MultiByteToWideChar" << "\n random character " << gsl::narrow_cast<int>(randElem16)
              << "\n length " << length << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    GetBasicDuration();
    std::unique_ptr<wchar_t[]> u16Buffer2{ std::make_unique<wchar_t[]>(u8Str.length()) };
    status = p_RtlUTF8ToUnicodeN(u16Buffer2.get(), gsl::narrow_cast<ULONG>(u8Str.length() * sizeof(wchar_t)), &written, u8Str.c_str(), gsl::narrow_cast<ULONG>(u8Str.length()));
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, gsl::narrow_cast<size_t>(written / sizeof(wchar_t))};
    [[gsl::suppress(26446)]] randElem16 = u16Buffer2[distribution(generator)];
    u16Buffer2.reset();
    std::cout << "RtlUTF8ToUnicodeN" << "\n random character " << gsl::narrow_cast<int>(randElem16)
              << "\n NTSTATUS " << status << "\n length " << (written / sizeof(wchar_t)) << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    GetBasicDuration();
    std::wstring u16Str{};
    hRes = U8ToU16(u8Str, u16Str);
    duration = GetBasicDuration();
    distribution = std::uniform_int_distribution<size_t>{0, u16Str.length()};
    randElem16 = u16Str.at(distribution(generator));
    std::cout << "U8ToU16"<< "\n random character " << gsl::narrow_cast<int>(randElem16)
              << "\n HRESULT " << hRes << "\n length " << u16Str.length() << "\n elapsed " << duration << "\n~~~~~" << std::endl;

    std::cout.setf(std::ios::hex, std::ios::basefield);
    std::cout.setf(std::ios::showbase);

    // clang-format off
    std::wstring u16Str1{
        gsl::narrow_cast<wchar_t>(0xD853), gsl::narrow_cast<wchar_t>(0xDF5C), // CJK UNIFIED IDEOGRAPH-24F5C
        gsl::narrow_cast<wchar_t>(0xD853) /*, gsl::narrow_cast<wchar_t>(0xDF5C)*/ // CJK UNIFIED IDEOGRAPH-24F5C
    };
    std::wstring u16Str2{
        /*gsl::narrow_cast<wchar_t>(0xD853), gsl::narrow_cast<wchar_t>(0xDF5C), // CJK UNIFIED IDEOGRAPH-24F5C
        gsl::narrow_cast<wchar_t>(0xD853),*/ gsl::narrow_cast<wchar_t>(0xDF5C) // CJK UNIFIED IDEOGRAPH-24F5C
    };
    // clang-format on

    std::cout << "\n*** UTF16ChunkToUTF8Converter ***" << std::endl;
    UTF16ChunkToUTF8Converter convertUTF16ChunkToUTF8{};
    std::string_view u8SvOut1{};
    std::cout << "HRESULT " << convertUTF16ChunkToUTF8(u16Str1, u8SvOut1) << std::endl;
    for (const auto& elem : u8SvOut1)
        std::cout << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(elem)) << ' ';
    std::cout << std::endl;
    std::string_view u8SvOut2{};
    std::cout << "HRESULT " << convertUTF16ChunkToUTF8(u16Str2, u8SvOut2) << std::endl;
    for (const auto& elem : u8SvOut2)
        std::cout << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(elem)) << ' ';
    std::cout << std::endl;

    std::cout << "\n*** UTF16PartialHandler ***" << std::endl;
    UTF16PartialHandler handleU16Partials{};
    std::wstring_view u16Sv1{ u16Str1 };
    std::cout << "HRESULT " << handleU16Partials(u16Sv1) << std::endl;
    for (const auto& elem : u16Sv1)
        std::cout << gsl::narrow_cast<int>(elem) << ' ';
    std::cout << std::endl;
    std::wstring_view u16Sv2{ u16Str2 };
    std::cout << "HRESULT " << handleU16Partials(u16Sv2) << std::endl;
    for (const auto& elem : u16Sv2)
        std::cout << gsl::narrow_cast<int>(elem) << ' ';
    std::cout << std::endl;

    // clang-format off
    std::string u8Str1{
        '\x7E', // TILDE
        '\xC3', '\xB6', // LATIN SMALL LETTER O WITH DIAERESIS
        '\xE2', '\x82', '\xAC', // EURO SIGN
        '\xF0' /*, '\xA4', '\xBD', '\x9C'*/ // CJK UNIFIED IDEOGRAPH-24F5C
    };
    std::string u8Str2{
        /*'\x7E', // TILDE
        '\xC3', '\xB6', // LATIN SMALL LETTER O WITH DIAERESIS
        '\xE2', '\x82', '\xAC', // EURO SIGN
        '\xF0',*/ '\xA4', '\xBD', '\x9C' // CJK UNIFIED IDEOGRAPH-24F5C
    };
    // clang-format on

    std::cout << "\n*** UTF8ChunkToUTF16Converter ***" << std::endl;
    UTF8ChunkToUTF16Converter convertUTF8ChunkToUTF16{};
    std::wstring_view u16SvOut1{};
    std::cout << "HRESULT " << convertUTF8ChunkToUTF16(u8Str1, u16SvOut1) << std::endl;
    for (const auto& elem : u16SvOut1)
        std::cout << gsl::narrow_cast<int>(elem) << ' ';
    std::cout << std::endl;
    std::wstring_view u16SvOut2{};
    std::cout << "HRESULT " << convertUTF8ChunkToUTF16(u8Str2, u16SvOut2) << std::endl;
    for (const auto& elem : u16SvOut2)
        std::cout << gsl::narrow_cast<int>(elem) << ' ';
    std::cout << std::endl;

    std::cout << "\n*** UTF8PartialHandler ***" << std::endl;
    UTF8PartialHandler handleU8Partials{};
    std::string_view u8Sv1{ u8Str1 };
    std::cout << "HRESULT " << handleU8Partials(u8Sv1) << std::endl;
    for (const auto& elem : u8Sv1)
        std::cout << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(elem)) << ' ';
    std::cout << std::endl;
    std::string_view u8Sv2{ u8Str2 };
    std::cout << "HRESULT " << handleU8Partials(u8Sv2) << std::endl;
    for (const auto& elem : u8Sv2)
        std::cout << gsl::narrow_cast<int>(gsl::narrow_cast<unsigned char>(elem)) << ' ';
    std::cout << std::endl;

    if (ntdll != nullptr)
    {
        FreeLibrary(ntdll);
    }

    return 0;
}
