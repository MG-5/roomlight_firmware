namespace util::wrappers
{

class ITaskWithMemberFunction
{
public:
    virtual void taskMain(void *parameters) = 0;
};

} // namespace util::wrappers