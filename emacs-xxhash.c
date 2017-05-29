#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "emacs-module.h"
#include "xxhash.h"

typedef int stallman_t;
stallman_t plugin_is_GPL_compatible;

#define assert_funcall(env, fn, argc, argv, errval)                     \
  do {                                                                  \
    emacs_value fn_res = funcall(env, fn, argc, argv);                  \
    if(!env->is_not_nil(env, fn_res)) {                                 \
      env->non_local_exit_throw(env, env->intern(env, "xxh-throw"), env->make_string(env, errval, strlen(errval))); \
    }                                                                   \
  } while(0);

static emacs_value funcall(emacs_env* env, const char* fn, int argc, emacs_value* argv) {
  emacs_value fn_sym = env->intern(env, fn);
  emacs_value fn_args[argc];
  for(int i = 0; i < argc; i++) {
    fn_args[i] = argv[i];
  }
  return env->funcall(env, fn_sym, argc, fn_args);
}

const char* const xxh_64_doc = "Return a hex string representing the 64-bit hash value of the given value. Its argument must be a vector of numbers less than 255.";
static emacs_value xxh_64(emacs_env* env, ptrdiff_t argc, emacs_value* argv, void* data) {
  // Ensure the argument is a vector
  assert_funcall(env, "vectorp", 1, argv, "That's not a vector!");

  XXH64_hash_t hash_result;
  { // Extract the values from the vector into a buffer
    const size_t buf_len = env->vec_size(env, argv[0]);
    unsigned char* const buf = calloc(sizeof(unsigned char), buf_len);
    for(size_t i = 0; i < buf_len; i++) {
      emacs_value num = env->vec_get(env, argv[0], i);

      if(!env->is_not_nil(env, funcall(env, "number-or-marker-p", 1, &num))) {
        free(buf);
        return env->make_string(env, "That's not a number!", strlen("That's not a number!"));
      }

      buf[i] = (unsigned char)(env->extract_integer(env, num) & 0xFF);
    }

    // Hash the value
    hash_result = XXH64((void*)buf, buf_len, 0);
    free(buf);
  }

  // Copy the hash into a hex string
  const size_t hex_str_len = sizeof(hash_result) * 2;
  char hex_str[hex_str_len];
  snprintf(hex_str, hex_str_len, "%0llX", hash_result);

  return env->make_string(env, hex_str, strlen(hex_str));
}

const char* const xxh_str_64_doc = "Return a hex string representing the 64-bit hash value of the given value. Its argument must be a string.";
static emacs_value xxh_str_64(emacs_env* env, ptrdiff_t argc, emacs_value* argv, void* data) {
  // Ensure the argument is a string
  assert_funcall(env, "stringp", 1, argv, "That's not a string!");

  long buf_len = 1024 * 16;
  char buf[buf_len];

  // Turn the emacs string into a char*
  if(!env->copy_string_contents(env, argv[0], buf, &buf_len)) {
    return env->make_string(env, "That string is too long.", strlen("That string is too long."));
  }

  // Hash the value
  XXH64_hash_t hash_result = XXH64((void*)buf, buf_len, 0);

  // Copy the hash into a hex string
  const size_t hex_str_len = sizeof(hash_result) * 2;
  char hex_str[hex_str_len];
  snprintf(hex_str, hex_str_len, "%0llX", hash_result);

  return env->make_string(env, hex_str, strlen(hex_str));
}

const char* const xxh_32_doc = "Return a hex string representing the 32-bit hash value of the given value. Its argument must be a vector of numbers less than 255.";
static emacs_value xxh_32(emacs_env* env, ptrdiff_t argc, emacs_value* argv, void* data) {
  // Ensure the argument is a vector
  assert_funcall(env, "vectorp", 1, argv, "That's not a vector!");

  XXH32_hash_t hash_result;
  { // Extract the values from the vector into a buffer
    const size_t buf_len = env->vec_size(env, argv[0]);
    unsigned char* const buf = calloc(sizeof(unsigned char), buf_len);
    for(size_t i = 0; i < buf_len; i++) {
      emacs_value num = env->vec_get(env, argv[0], i);

      if(!env->is_not_nil(env, funcall(env, "number-or-marker-p", 1, &num))) {
        free(buf);
        return env->make_string(env, "That's not a number!", strlen("That's not a number!"));
      }

      buf[i] = (unsigned char)(env->extract_integer(env, num) & 0xFF);
    }

    // Hash the value
    hash_result = XXH32((void*)buf, buf_len, 0);
    free(buf);
  }

  // Copy the hash into a hex string
  const size_t hex_str_len = sizeof(hash_result) * 2;
  char hex_str[hex_str_len];
  snprintf(hex_str, hex_str_len, "%0X", hash_result);

  return env->make_string(env, hex_str, strlen(hex_str));
}

const char* const xxh_str_32_doc = "Return a hex string representing the 32-bit hash value of the given value. Its argument must be a string.";
static emacs_value xxh_str_32(emacs_env* env, ptrdiff_t argc, emacs_value* argv, void* data) {
  // Ensure the argument is a string
  assert_funcall(env, "stringp", 1, argv, "That's not a string!");

  long buf_len = 1024 * 16;
  char buf[buf_len];

  // Turn the emacs string into a char*
  if(!env->copy_string_contents(env, argv[0], buf, &buf_len)) {
    return env->make_string(env, "That string is too long.", strlen("That string is too long."));
  }

  // Hash the value
  XXH32_hash_t hash_result = XXH32((void*)buf, buf_len, 0);

  // Copy the hash into a hex string
  const size_t hex_str_len = sizeof(hash_result) * 2;
  char hex_str[hex_str_len];
  snprintf(hex_str, hex_str_len, "%0X", hash_result);

  return env->make_string(env, hex_str, strlen(hex_str));
}


int emacs_module_init (struct emacs_runtime *ert) {
  emacs_env *env = ert->get_environment (ert);
  emacs_value fset = env->intern(env, "fset");

  // (defun xxh-64 (vec-of-numbers) ...)
  emacs_value xxh_64_sym = env->intern(env, "xxh-64");
  emacs_value xxh_64_fn = env->make_function(env, 1, 1, xxh_64, xxh_64_doc, NULL);
  emacs_value xxh64_args[] = { xxh_64_sym, xxh_64_fn };
  env->funcall(env, fset, 2, xxh64_args);

  // (defun xxh-64-str (string) ...)
  emacs_value xxh_str_64_sym = env->intern(env, "xxh-64-str");
  emacs_value xxh_str_64_fn = env->make_function(env, 1, 1, xxh_str_64, xxh_str_64_doc, NULL);
  emacs_value xxh64_str_args[] = { xxh_str_64_sym, xxh_str_64_fn };
  env->funcall(env, fset, 2, xxh64_str_args);

  // (defun xxh-32 (vec-of-numbers) ...)
  emacs_value xxh_32_sym = env->intern(env, "xxh-32");
  emacs_value xxh_32_fn = env->make_function(env, 1, 1, xxh_32, xxh_32_doc, NULL);
  emacs_value xxh32_args[] = { xxh_32_sym, xxh_32_fn };
  env->funcall(env, fset, 2, xxh32_args);

  // (defun xxh-32-str (string) ...)
  emacs_value xxh_str_32_sym = env->intern(env, "xxh-32-str");
  emacs_value xxh_str_32_fn = env->make_function(env, 1, 1, xxh_str_32, xxh_str_32_doc, NULL);
  emacs_value xxh32_str_args[] = { xxh_str_32_sym, xxh_str_32_fn };
  env->funcall(env, fset, 2, xxh32_str_args);

  // (provide 'xxh)
  emacs_value feature = env->intern(env, "xxh");
  emacs_value provide = env->intern(env, "provide");
  emacs_value provide_args[] = { feature };
  env->funcall(env, provide, 1, provide_args);

  return 0;
}
