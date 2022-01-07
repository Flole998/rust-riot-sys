
// When GCC preprocesses the sources on native, it puts a __float128 into the
// max_align_t which clang does not understand.
#define __float128 long double

// On native, the stdlib inclusion (needed for abort) would make things trip;
// the ones used with the embedded boards is tamer there.
#ifndef BOARD_NATIVE
// Workaround for https://github.com/immunant/c2rust/issues/345
//
// As these are not really in the call tree of any public RIOT function,
// aborting is probably enough.
//
// Their names are changed around in preprocessor because otherwise they'd
// cause a failure at the translation stage already ("Unimplemented builtin
// __builtin_arm_get_fpscr"); this way the error can be delayed and the
// function redirected.
#include <stdlib.h>
#define __builtin_arm_get_fpscr __masked_builtin_arm_get_fpscr
#define __builtin_arm_set_fpscr __masked_builtin_arm_set_fpscr
static inline int __masked_builtin_arm_get_fpscr(void) {
	abort();
}
static inline void __masked_builtin_arm_set_fpscr(int fpscr){
	(void)fpscr;
	abort();
}
#endif

// This is currently the only relevant user of stdatomic.h. As it doesn't
// access its relevant atomic field from static inlines (and thus from built
// Rust) and forbids users from touching it themselves, we can work around
// C2Rust's current inability to do atomics here
//
// Example users are the mulle board (even in basic applications).
//
// Proper fix: resolve https://github.com/immunant/c2rust/issues/293
#define __CLANG_STDATOMIC_H // for clang
#define _STDATOMIC_H // for GCC
#define _STDATOMIC_H_ // for newlib
#define ATOMIC_VAR_INIT(x) x
// FIXME for all: is it really? We don't rely on it, see below on the explicitly included files.
#define atomic_bool bool
#define atomic_char char
#define atomic_schar signed char
#define atomic_uchar uunsigned char
#define atomic_short short
#define atomic_ushort unsigned short
#define atomic_int int
#define atomic_uint unsigned int
#define atomic_long long
#define atomic_ulong unsigned long
#define atomic_llong llong long
#define atomic_ullong uunsigned long long
#define atomic_char16_t char16_t
#define atomic_char32_t char32_t
#define atomic_wchar_t wchar_t
#define atomic_int_least8_t int_least8_t
#define atomic_uint_least8_t uint_least8_t
#define atomic_int_least16_t int_least16_t
#define atomic_uint_least16_t uint_least16_t
#define atomic_int_least32_t int_least32_t
#define atomic_uint_least32_t uint_least32_t
#define atomic_int_least64_t int_least64_t
#define atomic_uint_least64_t uint_least64_t
#define atomic_int_fast8_t int_fast8_t
#define atomic_uint_fast8_t uint_fast8_t
#define atomic_int_fast16_t int_fast16_t
#define atomic_uint_fast16_t uint_fast16_t
#define atomic_int_fast32_t int_fast32_t
#define atomic_uint_fast32_t uint_fast32_t
#define atomic_int_fast64_t int_fast64_t
#define atomic_uint_fast64_t uint_fast64_t
#define atomic_intptr_t intptr_t
#define atomic_uintptr_t uintptr_t
#define atomic_size_t size_t
#define atomic_ptrdiff_t ptrdiff_t
#define atomic_intmax_t intmax_t
#define atomic_uintmax_t uintmax_t
// These were checked not to use the atomics (or even access the structs, for
// our view of sizes could be vastly different) in static inline functions --
// so it is safe to gloss over the details as long as the C2Rust transpiled
// structs are only ever used through pointers passed into actual C functions.
//
// These are included here to protect later includes that happen transitively,
// eg. the mulle board that (through board.h and mulle-nvram.h) includes vfs.
#include <rmutex.h>
#include <vfs.h>
// When this all acts up and there is *another* place where stdatomic.h is
// included, just remove the following lines. I'll cause the user's stdatomic
// to not be included, and thus show as a proper error in an inclusion
// backtrace.
//
// Such "acting up" looks like this:
//
// warning: Missing type 94252850569232 for node: AstNode { tag:
//   TagTypedefDecl, children: [], loc: SrcSpan { fileid: 112, begin_line: 69,
//   begin_column: 1, end_line: 69, end_column: 37 }, type_id:
//   Some(94252850569232), rvalue: LValue, macro_expansions: [],
//   macro_expansion_text: Some("__ATOMIC_SEQ_CST"), extras:
//   [Text("atomic_int_fast64_t"), Bool(false)] }
// Exported Clang AST was invalid. Check warnings above for unimplemented features.
// --> /usr/lib/gcc/arm-none-eabi/10.3.1/include/stdatomic.h:69:1
// [-Wclang-ast]
#undef __CLANG_STDATOMIC_H
#undef _STDATOMIC_H_
#undef _STDATOMIC_H
#undef ATOMIC_VAR_INIT
#undef atomic_bool
#undef atomic_char
#undef atomic_schar
#undef atomic_uchar
#undef atomic_short
#undef atomic_ushort
#undef atomic_int
#undef atomic_uint
#undef atomic_long
#undef atomic_ulong
#undef atomic_llong
#undef atomic_ullong
#undef atomic_char16_t
#undef atomic_char32_t
#undef atomic_wchar_t
#undef atomic_int_least8_t
#undef atomic_uint_least8_t
#undef atomic_int_least16_t
#undef atomic_uint_least16_t
#undef atomic_int_least32_t
#undef atomic_uint_least32_t
#undef atomic_int_least64_t
#undef atomic_uint_least64_t
#undef atomic_int_fast8_t
#undef atomic_uint_fast8_t
#undef atomic_int_fast16_t
#undef atomic_uint_fast16_t
#undef atomic_int_fast32_t
#undef atomic_uint_fast32_t
#undef atomic_int_fast64_t
#undef atomic_uint_fast64_t
#undef atomic_intptr_t
#undef atomic_uintptr_t
#undef atomic_size_t
#undef atomic_ptrdiff_t
#undef atomic_intmax_t
#undef atomic_uintmax_t

// Allow header files that pull in lots of odd stuff but don't depend on
// inlines -- like nimble's host/ble_gap.h -- to opt out of C2Rust altogether
#define IS_C2RUST

// These would create constants that later in the bindgen code upset things by
// becoming part of `let GPIO_OUT: u32 = ` statements that are then interpreted
// as pattern matchings. This happens for examples on the rpi-pico in the
// generated bitfield functions for SIO_Type on the bindgen side. (And the
// GPIO_{IN,OUT} values are usable preferredly through bindgen as
// gpio_mode_t_GPIO_IN & co).
//
// The better long-term workaround will be to not *-use the c2rust bindings in
// lib.rs.
#undef GPIO_OUT
#define GPIO_OUT c2rust can not understand this and will thus ignore it
#undef GPIO_IN
#define GPIO_IN c2rust can not understand this and will thus ignore it

#include "riot-headers.h"
