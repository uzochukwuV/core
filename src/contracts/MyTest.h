using namespace QPI;

struct MYTEST2
{
};

struct MYTEST : public ContractBase
{
public:
    struct add_input
    {
        sint64 a;
        sint64 b;
    };

    struct add_output
    {
        sint64 out;
    };

    PUBLIC_FUNCTION(add)
    {
        output.out = input.a + input.b;
    }

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_FUNCTION(add, 1);
    }
};
