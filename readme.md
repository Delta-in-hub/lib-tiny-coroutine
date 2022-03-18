# Simple Coroutine In C language
A Simple Coroutine demo.
Header only.
Only support x86/64 yet,but not support Windows.
Thanks to nju/os-course.
## API
 * `struct coroutine *cocreate(void (*entry)(void *), void *arg);`
   * @brief create a new coroutine , it's not running now but be ready for it.
   * @param entry function pointer of the coroutine, void (*entry)(void *)
   * @param arg argument of the coroutine


 * `void coyield();`
   * @brief curent coroutine yield and switch an avaiable coroutine.

 * `void cowait(struct coroutine *co);`
    * @brief curent coroutine yield and could run again till watting-coroutine is finished.
    * @param coroutine coroutine to be waited

## Example
```bash
> make test
> less result.log
```






