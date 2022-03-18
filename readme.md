# Simple Coroutine In C language
A simple demo for learning.
Header only.
Only support x86/64 yet,but not support Windows.
Thanks to nju/os-course.
## API
 * `struct coroutine *cocreate(void (*entry)(void *), void *arg);`
   * @brief append a new-created coroutine to the coroutine list
   * @param coroutine coroutine to be appended

 * `void coyield();`
   * @brief curent coroutine yield and switch an avaiable coroutine.

 * `void cowait(struct coroutine *co);`
    * @brief curent coroutine yield and could run again till watting-coroutine is finished.
    * @param coroutine coroutine to be waited






