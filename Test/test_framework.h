#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

/*===========================================================================*
 * FS2026 MİNİMAL TEST FRAMEWORK
 *===========================================================================*
 *
 *  Dışarıdan kütüphane kullanmadan, sıfırdan yazılmış basit bir test çerçevesi.
 *  Bilgisayarda gcc ile derlenir ve çalıştırılır.
 *
 *  Kullanım:
 *    TEST_ASSERT(koşul, "Açıklama");     → Koşul false ise testi FAIL yapar
 *    TEST_ASSERT_EQ(a, b, "Açıklama");   → a == b değilse FAIL
 *    TEST_ASSERT_NEQ(a, b, "Açıklama");  → a != b değilse FAIL
 *
 *===========================================================================*/

// Renkli çıktılar (Terminal için)
#define COLOR_GREEN  "\033[32m"
#define COLOR_RED    "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET  "\033[0m"

// Global sayaçlar
static int test_passed = 0;
static int test_failed = 0;
static int test_total  = 0;

// Test bloğu başlangıcı (Görsel ayraç)
#define TEST_SUITE_BEGIN(name) \
    printf("\n" COLOR_YELLOW "═══════════════════════════════════════════════\n"); \
    printf("  TEST SÜİTİ: %s\n", name); \
    printf("═══════════════════════════════════════════════" COLOR_RESET "\n");

// Basit assert: Koşul doğru mu?
#define TEST_ASSERT(condition, description) do { \
    test_total++; \
    if (condition) { \
        test_passed++; \
        printf(COLOR_GREEN "  [PASS] " COLOR_RESET "%s\n", description); \
    } else { \
        test_failed++; \
        printf(COLOR_RED   "  [FAIL] " COLOR_RESET "%s  (Satır: %d)\n", description, __LINE__); \
    } \
} while(0)

// İki değer eşit mi?
#define TEST_ASSERT_EQ(actual, expected, description) do { \
    test_total++; \
    if ((actual) == (expected)) { \
        test_passed++; \
        printf(COLOR_GREEN "  [PASS] " COLOR_RESET "%s\n", description); \
    } else { \
        test_failed++; \
        printf(COLOR_RED   "  [FAIL] " COLOR_RESET "%s  (Beklenen: %lld, Gelen: %lld, Satır: %d)\n", \
               description, (long long)(expected), (long long)(actual), __LINE__); \
    } \
} while(0)

// İki değer farklı mı?
#define TEST_ASSERT_NEQ(actual, not_expected, description) do { \
    test_total++; \
    if ((actual) != (not_expected)) { \
        test_passed++; \
        printf(COLOR_GREEN "  [PASS] " COLOR_RESET "%s\n", description); \
    } else { \
        test_failed++; \
        printf(COLOR_RED   "  [FAIL] " COLOR_RESET "%s  (Değer %lld olmamalıydı, Satır: %d)\n", \
               description, (long long)(not_expected), __LINE__); \
    } \
} while(0)

// Sonuç raporu
#define TEST_REPORT() do { \
    printf("\n" COLOR_YELLOW "═══════════════════════════════════════════════\n"); \
    printf("  SONUÇ RAPORU\n"); \
    printf("═══════════════════════════════════════════════" COLOR_RESET "\n"); \
    printf("  Toplam: %d | ", test_total); \
    printf(COLOR_GREEN "Geçen: %d" COLOR_RESET " | ", test_passed); \
    if (test_failed > 0) { \
        printf(COLOR_RED "Kalan: %d" COLOR_RESET, test_failed); \
    } else { \
        printf(COLOR_GREEN "Kalan: 0 — TÜM TESTLER GEÇTİ! 🎉" COLOR_RESET); \
    } \
    printf("\n\n"); \
} while(0)

#endif // TEST_FRAMEWORK_H
