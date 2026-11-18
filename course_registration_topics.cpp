// =====================================================================
//  College Course Registration System - OOPD Topic Demo (C++17)
//
//  Build : g++ -std=c++17 course_registration_topics.cpp -o topics
//  Run   : ./topics
//
//  Each PART below covers one group of topics, in syllabus order.
//  Some Java topics (throws, finally) do not exist as keywords in C++,
//  so the closest C++ equivalent is used and explained in a comment.
// =====================================================================
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <functional>
using namespace std;

// ---------- small helper used by all parts ----------
void heading(const string& title) {
    cout << "\n=====================================================\n"
         << title
         << "\n=====================================================\n";
}

// =====================================================================
// PART 1: Inheritance, Access Specifiers, Constructors,
//         Operator Overloading (member), Method Overloading
// =====================================================================
class Person {
private:                               // private  : only Person can use it
    int id;
protected:                             // protected: Person + derived classes
    string name;
public:                                // public   : everyone
    // Constructors: default, parameterised, copy
    Person() : id(0), name("Unknown") {
        cout << "  [Person] default constructor\n";
    }
    Person(int id, const string& name) : id(id), name(name) {
        cout << "  [Person] parameterised constructor\n";
    }
    Person(const Person& other) : id(other.id), name(other.name) {
        cout << "  [Person] copy constructor\n";
    }
    virtual ~Person() {}               // virtual destructor (needed for polymorphism)

    int getId() const { return id; }
    string getName() const { return name; }

    // Method overloading: same name, different parameter lists
    void display() const {
        cout << "  " << id << " - " << name << "\n";
    }
    void display(const string& label) const {
        cout << "  " << label << ": " << id << " - " << name << "\n";
    }
    void display(const string& label, int times) const {
        for (int i = 0; i < times; i++) display(label);
    }

    virtual void showMenu() const { cout << "  [Person] generic menu\n"; }
    virtual string role() const { return "Person"; }
};

// Single inheritance: Student IS-A Person
class Student : public Person {
private:
    int credits;
    vector<string> courses;
public:
    Student() : Person(), credits(0) {
        cout << "  [Student] default constructor\n";
    }
    Student(int id, const string& name, int credits = 0)
        : Person(id, name), credits(credits) {
        cout << "  [Student] parameterised constructor\n";
    }
    int getCredits() const { return credits; }

    // Method overloading
    void registerCourse(const string& code) {
        courses.push_back(code);
        credits += 3;
        cout << "  " << name << " registered " << code << " (default 3 credits)\n";
    }
    void registerCourse(const string& code, int c) {
        courses.push_back(code);
        credits += c;
        cout << "  " << name << " registered " << code << " (" << c << " credits)\n";
    }
    void registerCourse(const vector<string>& codes) {
        for (const string& c : codes) registerCourse(c);
    }

    // Operator overloading using member functions
    Student operator+(int extraCredits) const {     // s + 3
        Student temp = *this;
        temp.credits += extraCredits;
        return temp;
    }
    bool operator>(const Student& other) const {    // s1 > s2
        return credits > other.credits;
    }
    Student& operator++() {                         // ++s  (prefix)
        ++credits;
        return *this;
    }
    Student operator++(int) {                       // s++  (postfix)
        Student old = *this;
        ++credits;
        return old;
    }

    void showMenu() const override {
        cout << "  [Student] 1.Register course 2.Drop course 3.View fees\n";
    }
    string role() const override { return "Student"; }
};

// Multilevel inheritance: PGStudent -> Student -> Person
class PGStudent : public Student {
private:
    string thesis;
public:
    PGStudent(int id, const string& name, int credits, const string& thesis)
        : Student(id, name, credits), thesis(thesis) {
        cout << "  [PGStudent] parameterised constructor\n";
    }
    string getThesis() const { return thesis; }
    void showMenu() const override {
        cout << "  [PGStudent] 1.Register course 2.Submit thesis 3.View guide\n";
    }
    string role() const override { return "PG Student"; }
};

// Hierarchical inheritance: Faculty also derives from Person
class Faculty : public Person {
private:
    string department;
public:
    Faculty(int id, const string& name, const string& dept)
        : Person(id, name), department(dept) {
        cout << "  [Faculty] parameterised constructor\n";
    }
    void showMenu() const override {
        cout << "  [Faculty] 1.View class list 2.Upload marks\n";
    }
    string role() const override { return "Faculty (" + department + ")"; }
};

void part1_inheritance_and_basics() {
    heading("PART 1: Inheritance, Access Specifiers, Constructors,\n"
            "        Operator Overloading, Method Overloading");

    cout << "-- Constructors (default, parameterised, copy) --\n";
    Student s0;
    Student s1(101, "Aarav", 6);
    Student s2 = s1;                   // copy constructor

    cout << "-- Inheritance + access specifiers --\n";
    s1.display();
    // s1.id = 5;       // ERROR: 'id' is private in Person
    // s1.name = "X";   // ERROR: 'name' is protected (only Person and derived classes)
    cout << "  Public getter works: " << s1.getName() << "\n";

    cout << "-- Method overloading --\n";
    s1.display("Student");
    s1.display("Repeat", 2);
    s1.registerCourse("CS201");
    s1.registerCourse("MA202", 4);
    s1.registerCourse(vector<string>{"EC203", "HS204"});

    cout << "-- Operator overloading (member functions) --\n";
    Student s3 = s1 + 3;
    cout << "  s1 credits = " << s1.getCredits()
         << ", s3 = s1 + 3 -> " << s3.getCredits() << "\n";
    cout << "  s3 > s1 ? " << (s3 > s1 ? "yes" : "no") << "\n";
    ++s1;
    s1++;
    cout << "  after ++s1 and s1++ : " << s1.getCredits() << "\n";
}

// =====================================================================
// PART 2: Operator Overloading using a FRIEND function
// =====================================================================
class Fee {
private:
    int amount;
public:
    explicit Fee(int a = 0) : amount(a) {}
    int get() const { return amount; }

    // Friends are NOT members, but can touch private data.
    friend Fee operator+(const Fee& a, const Fee& b);
    friend Fee operator+(int a, const Fee& b);       // 500 + fee  (cannot be a member!)
    friend bool operator==(const Fee& a, const Fee& b);
};
Fee operator+(const Fee& a, const Fee& b) { return Fee(a.amount + b.amount); }
Fee operator+(int a, const Fee& b)        { return Fee(a + b.amount); }
bool operator==(const Fee& a, const Fee& b) { return a.amount == b.amount; }

void part2_friend_operators() {
    heading("PART 2: Operator Overloading using friend function");
    Fee tuition(40000), hostel(15000);
    Fee total = tuition + hostel;
    Fee withLateFine = 500 + total;    // left operand is an int -> needs a friend
    cout << "  tuition + hostel      = " << total.get() << "\n";
    cout << "  500 + total           = " << withLateFine.get() << "\n";
    cout << "  total == Fee(55000) ? " << (total == Fee(55000) ? "yes" : "no") << "\n";
}

// =====================================================================
// PART 3: Stream operators (<< and >>) overloaded with friend functions
// =====================================================================
class Course {
private:
    string code, name;
    int credits;
public:
    Course(const string& c = "", const string& n = "", int cr = 0)
        : code(c), name(n), credits(cr) {}
    string getCode() const { return code; }
    int getCredits() const { return credits; }

    friend ostream& operator<<(ostream& os, const Course& c);
    friend istream& operator>>(istream& is, Course& c);
};
ostream& operator<<(ostream& os, const Course& c) {
    return os << c.code << " | " << c.name << " | " << c.credits << " credits";
}
istream& operator>>(istream& is, Course& c) {
    return is >> c.code >> c.name >> c.credits;
}

void part3_stream_operators() {
    heading("PART 3: Stream operators << and >> (friend functions)");
    Course c1("CS201", "OOPD", 4);
    cout << "  Output with <<: " << c1 << "\n";

    istringstream input("EC210 Signals 3");   // pretend this is keyboard input
    Course c2;
    input >> c2;                              // for real input use: cin >> c2;
    cout << "  Read with >>  : " << c2 << "\n";
}

// =====================================================================
// PART 4: Nested class and Object slicing
// =====================================================================
class Timetable {
public:
    class Slot {                              // nested class
    public:
        string day;
        int hour;
        Slot(const string& d, int h) : day(d), hour(h) {}
        void show() const { cout << "    " << day << " at " << hour << ":00\n"; }
    };
private:
    vector<Slot> slots;
public:
    void addSlot(const string& d, int h) { slots.emplace_back(d, h); }
    void show() const {
        for (const Slot& s : slots) s.show();
    }
};

void part4_nested_and_slicing() {
    heading("PART 4: Nested class, Object slicing");

    cout << "-- Nested class --\n";
    Timetable tt;
    tt.addSlot("Mon", 9);
    tt.addSlot("Wed", 11);
    tt.show();
    Timetable::Slot extra("Fri", 14);         // nested class used from outside
    extra.show();

    cout << "-- Object slicing --\n";
    PGStudent pg(201, "Meera", 12, "AI in Education");
    Person sliced = pg;                       // only the Person part is copied; the rest is SLICED off
    cout << "  Sliced copy calls: ";
    sliced.showMenu();                        // Person's menu, even though showMenu is virtual
    Person& ref = pg;                         // a reference does NOT slice
    cout << "  Reference calls  : ";
    ref.showMenu();                           // PGStudent's menu
}

// =====================================================================
// PART 5: Static variables, Static functions, final
// =====================================================================
class Registration {
private:
    static int totalRegistrations;            // ONE copy shared by all objects
    int regId;
    string student, course;
public:
    Registration(const string& s, const string& c)
        : regId(++totalRegistrations), student(s), course(c) {}
    void show() const {
        cout << "  Reg#" << regId << ": " << student << " -> " << course << "\n";
    }
    static int getTotal() { return totalRegistrations; }   // static function: no 'this'
};
int Registration::totalRegistrations = 0;     // static member must be defined outside

int nextReceiptNo() {
    static int counter = 1000;                // static LOCAL variable: keeps its value between calls
    return ++counter;
}

class Report {
public:
    virtual ~Report() = default;
    virtual void print() const { cout << "  Generic report\n"; }
    virtual void title() const final {        // 'final' method: derived classes cannot override
        cout << "  Report title (cannot be overridden)\n";
    }
};
class SemesterReport final : public Report {  // 'final' class: cannot be inherited
public:
    void print() const override { cout << "  Semester report: registrations summary\n"; }
    // void title() const override {}         // ERROR: Report::title is final
};
// class MidSemReport : public SemesterReport {};   // ERROR: SemesterReport is final

void part5_static_and_final() {
    heading("PART 5: Static variables, Static functions, final");
    Registration r1("Aarav", "CS201"), r2("Meera", "MA202"), r3("Kabir", "EC203");
    r1.show(); r2.show(); r3.show();
    cout << "  Total registrations (static function): " << Registration::getTotal() << "\n";
    cout << "  Receipt numbers (static local): "
         << nextReceiptNo() << ", " << nextReceiptNo() << ", " << nextReceiptNo() << "\n";

    SemesterReport rep;
    rep.title();
    rep.print();
}

// =====================================================================
// PART 6: this pointer, Virtual functions, Virtual base class,
//         Pointers to objects and to derived classes
// =====================================================================
class Enrollment {
private:
    string studentName, courseCode;
    int semester;
public:
    Enrollment() : semester(0) {}
    // Parameter names equal member names, so 'this->' tells them apart.
    // Returning *this allows method chaining.
    Enrollment& setStudent(const string& studentName) { this->studentName = studentName; return *this; }
    Enrollment& setCourse(const string& courseCode)   { this->courseCode = courseCode;   return *this; }
    Enrollment& setSemester(int semester)             { this->semester = semester;       return *this; }
    bool isSameObject(const Enrollment& other) const  { return this == &other; }
    void show() const {
        cout << "  " << studentName << " | " << courseCode << " | sem " << semester
             << " | this = " << static_cast<const void*>(this) << "\n";
    }
};

// Virtual base class: solves the "diamond problem"
class Member {
protected:
    string name;
public:
    explicit Member(const string& n = "Unknown") : name(n) {
        cout << "  [Member] constructor (runs only ONCE)\n";
    }
};
class StudentMember : virtual public Member {
public:
    explicit StudentMember(const string& n = "Unknown") : Member(n) {
        cout << "  [StudentMember] constructor\n";
    }
};
class FacultyMember : virtual public Member {
public:
    explicit FacultyMember(const string& n = "Unknown") : Member(n) {
        cout << "  [FacultyMember] constructor\n";
    }
};
class TeachingAssistant : public StudentMember, public FacultyMember {
public:
    // The most-derived class initialises the virtual base directly.
    explicit TeachingAssistant(const string& n)
        : Member(n), StudentMember(n), FacultyMember(n) {
        cout << "  [TeachingAssistant] constructor\n";
    }
    void showName() const {
        // Without 'virtual' this would be ambiguous (two copies of 'name').
        cout << "  TA name from the single shared Member: " << name << "\n";
    }
};

void part6_this_virtual_pointers() {
    heading("PART 6: this pointer, Virtual function, Virtual base class,\n"
            "        Pointers to objects / derived classes");

    cout << "-- this pointer --\n";
    Enrollment e1, e2;
    e1.setStudent("Aarav").setCourse("CS201").setSemester(3);   // chaining via *this
    e2.setStudent("Meera").setCourse("MA202").setSemester(3);
    e1.show();
    e2.show();
    cout << "  e1 is e1 ? " << (e1.isSameObject(e1) ? "yes" : "no")
         << ", e1 is e2 ? " << (e1.isSameObject(e2) ? "yes" : "no") << "\n";

    cout << "-- Virtual function (runtime polymorphism) --\n";
    Student st(101, "Aarav", 6);
    PGStudent pg(201, "Meera", 12, "AI in Education");
    Faculty fac(900, "Dr. Rao", "CSE");
    Person* users[] = { &st, &pg, &fac };     // base-class pointers to different derived objects
    for (Person* p : users) {
        cout << "  Role: " << p->role() << "\n";
        p->showMenu();                        // the derived version runs
    }

    cout << "-- Pointers to objects and to derived classes --\n";
    Student* sp = new Student(105, "Kabir", 3);   // pointer to an object (heap)
    sp->registerCourse("CS201");
    delete sp;                                    // frees memory

    Person* bp = &pg;                             // base pointer -> derived object
    // bp->getThesis();                           // ERROR: Person pointer only sees Person members
    PGStudent* dp = dynamic_cast<PGStudent*>(bp); // safe downcast to the derived type
    if (dp) cout << "  Downcast OK, thesis = " << dp->getThesis() << "\n";
    Person* fp = &fac;                            // base pointer -> Faculty object
    Student* notStudent = dynamic_cast<Student*>(fp);   // wrong type, so the cast fails safely
    cout << "  Faculty downcast to Student gives " << (notStudent ? "object" : "nullptr") << "\n";

    cout << "-- Virtual base class --\n";
    TeachingAssistant ta("Kabir");
    ta.showName();
}

// =====================================================================
// PART 7: Type casting (basic<->class, class<->class) and Templates
// =====================================================================
class ContactHours {
private:
    int hours;
public:
    explicit ContactHours(int h = 0) : hours(h) {}
    int get() const { return hours; }
};

class Credits {
private:
    int value;
public:
    Credits(int v = 0) : value(v) {}                  // Basic -> Class (converting constructor)
    operator int() const { return value; }            // Class -> Basic (conversion operator)
    operator ContactHours() const {                   // Class -> Class (conversion operator)
        return ContactHours(value * 15);              // 1 credit = 15 contact hours
    }
};

template <typename T>                                 // function template
T maxOf(T a, T b) { return (a > b) ? a : b; }

template <typename T>                                 // class template
class Registry {
private:
    vector<T> items;
public:
    void add(const T& item) { items.push_back(item); }
    size_t size() const { return items.size(); }
    void showAll(const string& title) const {
        cout << "  " << title << " (" << items.size() << " items)\n";
        for (const T& item : items) cout << "    - " << item << "\n";
    }
};

void part7_casting_and_templates() {
    heading("PART 7: Type casting and Templates");

    cout << "-- Basic to Class --\n";
    Credits c = 4;                                    // int -> Credits
    cout << "  int 4 became a Credits object\n";

    cout << "-- Class to Basic --\n";
    int n = c;                                        // Credits -> int
    cout << "  Credits object as int = " << n << "\n";

    cout << "-- Class to Class --\n";
    ContactHours h = c;                               // Credits -> ContactHours
    cout << "  4 credits = " << h.get() << " contact hours\n";

    cout << "-- Function template --\n";
    cout << "  maxOf(3, 7)            = " << maxOf(3, 7) << "\n";
    cout << "  maxOf(2.5, 1.5)        = " << maxOf(2.5, 1.5) << "\n";
    cout << "  maxOf(\"CS\", \"EC\")      = " << maxOf(string("CS"), string("EC")) << "\n";

    cout << "-- Class template --\n";
    Registry<Course> courseList;
    courseList.add(Course("CS201", "OOPD", 4));
    courseList.add(Course("MA202", "Maths", 3));
    courseList.showAll("Registry<Course>");
    Registry<string> codes;
    codes.add("CS201");
    codes.add("EC203");
    codes.showAll("Registry<string>");
}

// =====================================================================
// PART 8: Exception handling
// =====================================================================
// User-defined exception hierarchy
class RegistrationException : public exception {
private:
    string msg;
public:
    explicit RegistrationException(const string& m) : msg(m) {}
    const char* what() const noexcept override { return msg.c_str(); }
};
class CourseFullException : public RegistrationException {
public:
    explicit CourseFullException(const string& code)
        : RegistrationException("Course " + code + " is full") {}
};
class PrerequisiteNotMetException : public RegistrationException {
public:
    PrerequisiteNotMetException(const string& code, const string& prereq)
        : RegistrationException("Cannot register " + code + ": prerequisite " + prereq + " not completed") {}
};
class FeeNotPaidException : public RegistrationException {
public:
    FeeNotPaidException() : RegistrationException("Fee not paid or not yet verified") {}
};

int feeFor(int credits) {
    if (credits <= 0) throw invalid_argument("credits must be positive");   // throw
    return credits * 5000;
}

// "throws" in C++: the old throw(Type) lists were removed in C++17.
// Today: noexcept(false) = may throw (the default), noexcept = promises not to.
void tryRegister(const string& code, bool feePaid, int seatsLeft, bool prereqDone) noexcept(false) {
    if (!feePaid)       throw FeeNotPaidException();
    if (seatsLeft <= 0) throw CourseFullException(code);
    if (!prereqDone)    throw PrerequisiteNotMetException(code, "CS101");
    cout << "  Registered for " << code << " successfully\n";
}
void safeLog(const string& m) noexcept { cout << "  [log] " << m << "\n"; }

// C++ has no 'finally' keyword. The idiom is RAII: a destructor always runs
// when the scope ends, whether or not an exception was thrown.
class Finally {
private:
    function<void()> action;
public:
    explicit Finally(function<void()> a) : action(move(a)) {}
    Finally(const Finally&) = delete;
    ~Finally() { action(); }
};

void part8_exceptions() {
    heading("PART 8: Exception Handling");

    cout << "-- try / catch / throw --\n";
    try {
        int fee4 = feeFor(4);
        cout << "  Fee for 4 credits = " << fee4 << "\n";
        int fee0 = feeFor(0);                         // throws here
        cout << "  Fee for 0 credits = " << fee0 << "\n";
        cout << "  (this line is skipped because the throw jumped to catch)\n";
    } catch (const invalid_argument& e) {
        cout << "  Caught invalid_argument: " << e.what() << "\n";
    }
    try { throw 42; }                                 // any type can be thrown
    catch (int code) { cout << "  Caught an int: " << code << "\n"; }

    cout << "-- 'throws' equivalent: noexcept(false) / noexcept --\n";
    safeLog("this function is declared noexcept, so it never throws");

    cout << "-- finally block (RAII guard) --\n";
    {
        Finally cleanup([] { cout << "  finally: cleanup ALWAYS runs\n"; });
        try { tryRegister("CS201", false, 5, true); }
        catch (const FeeNotPaidException& e) { cout << "  Caught: " << e.what() << "\n"; }
    }

    cout << "-- User-defined exceptions + base/derived catch order --\n";
    struct Attempt { const char* code; bool fee; int seats; bool prereq; };
    Attempt attempts[] = {
        {"CS201", true,  0, true },     // course full
        {"CS301", true,  5, false},     // prerequisite missing
        {"MA101", false, 5, true },     // fee not paid
        {"EC101", true,  5, true }      // success
    };
    for (const Attempt& a : attempts) {
        try {
            tryRegister(a.code, a.fee, a.seats, a.prereq);
        }
        // Order matters: put DERIVED handlers first, BASE handlers last.
        catch (const CourseFullException& e) {
            cout << "  [derived handler: CourseFullException] " << e.what() << "\n";
        }
        catch (const PrerequisiteNotMetException& e) {
            cout << "  [derived handler: PrerequisiteNotMetException] " << e.what() << "\n";
        }
        catch (const RegistrationException& e) {      // catches FeeNotPaidException too
            cout << "  [base handler: RegistrationException] " << e.what() << "\n";
        }
        catch (const exception& e) {                  // any standard exception
            cout << "  [std::exception] " << e.what() << "\n";
        }
        catch (...) {                                 // anything else
            cout << "  [catch-all] unknown error\n";
        }
        // Always catch by const reference. Catching the base class BY VALUE would slice.
    }

    cout << "-- Rethrow --\n";
    try {
        try { tryRegister("CS201", true, 0, true); }
        catch (const RegistrationException&) {
            cout << "  inner: logging, then rethrowing\n";
            throw;                                    // passes the same exception outward
        }
    } catch (const exception& e) {
        cout << "  outer caught: " << e.what() << "\n";
    }
}

// =====================================================================
int main() {
    part1_inheritance_and_basics();
    part2_friend_operators();
    part3_stream_operators();
    part4_nested_and_slicing();
    part5_static_and_final();
    part6_this_virtual_pointers();
    part7_casting_and_templates();
    part8_exceptions();
    cout << "\nAll topics demonstrated.\n";
    return 0;
}
