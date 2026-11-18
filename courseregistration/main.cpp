// College Course Registration System - web version, C++ only (NO JavaScript)
//
// The page layouts live in index.html / login.html; this program fills them with data. Every click is
// a normal link or form submit; the OOP classes below decide what happens.
// Uses only POSIX sockets (Linux / macOS / WSL) - no external library needed.
//
// Files:  main.cpp (this file), style.css, index.html, login.html  (keep in one folder)
//         data.txt is created automatically and keeps your data between restarts
// Build:  g++ -std=c++17 -O2 -o server main.cpp
// Run:    ./server            then open  http://localhost:8080
//         ./server 9000       to use another port
// Demo password for every account: 1234

// ============================ BUSINESS LOGIC (OOP model) ============================
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

// ---------------------------------------------------------------- exceptions
class RegistrationError : public runtime_error {
public:
    explicit RegistrationError(const string& m) : runtime_error(m) {}
};

// -------------------------------------------------------------------- Money
class Money {
    long v;
public:
    explicit Money(long value = 0) : v(value) {}
    long value() const { return v; }
    Money operator+(const Money& o) const { return Money(v + o.v); }
    bool operator==(const Money& o) const { return v == o.v; }
    static string inr(long v) {                      // Indian digit grouping, like toLocaleString('en-IN')
        string s = to_string(v < 0 ? -v : v), out;
        if (s.size() > 3) {
            string last = s.substr(s.size() - 3);
            s = s.substr(0, s.size() - 3);
            while (s.size() > 2) { out = "," + s.substr(s.size() - 2) + out; s = s.substr(0, s.size() - 2); }
            out = s + out + "," + last;
        } else out = s;
        return "₹" + out;
    }
    string str() const { return inr(v); }
    friend ostream& operator<<(ostream& os, const Money& m) { return os << m.str(); }
};

// ---------------------------------------------------------- generic helpers
template <typename Container, typename Pred>
auto findIf(Container& c, Pred p) -> decltype(&*c.begin()) {
    auto it = find_if(c.begin(), c.end(), p);
    return it == c.end() ? nullptr : &*it;
}

static string now() {
    time_t t = time(nullptr);
    char buf[32];
    strftime(buf, sizeof buf, "%d/%m/%Y, %I:%M:%S %p", localtime(&t));
    return buf;
}
static string upper(string s) { for (auto& c : s) c = toupper((unsigned char)c); return s; }

// ------------------------------------------------------------------- Course
struct Course {
    string code, name, dept;
    int sem, seats;
    bool listed;
};

// ------------------------------------------------------------ plain records
enum class Status { Pending, Approved, Rejected };
enum class Stage  { Advisor, HOD, Done };

static const char* toStr(Status s) {
    return s == Status::Pending ? "pending" : s == Status::Approved ? "approved" : "rejected";
}

struct Registration {
    int id;
    string sid, code;
    bool backlog;
    Status status = Status::Pending;
    Stage stage = Stage::Advisor;
    string reason;
};

struct Payment {
    Money amount;
    string method, ref, date;
    bool paid = false, verified = false, failed = false;
};

struct Notification { string uid, text, time; };

// ------------------------------------------------------------- PaymentGateway
class PaymentGateway {
public:
    // Demo rule: details shorter than 4 chars are invalid; ending in 0000 are declined.
    static string process(const string& details) {
        if (details.size() < 4) throw RegistrationError("Invalid payment details.");
        if (details.substr(details.size() - 4) == "0000")
            throw RegistrationError("Payment declined by the gateway.");
        auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        string t = to_string(ms);
        return "TXN" + t.substr(t.size() - 8);
    }
};

// ---------------------------------------------------------------- user types
class RegistrationSystem;

class User {
    string id_, name_, role_, password_;       // password is private (encapsulation)
public:
    User(string id, string name, string role, string pw)
        : id_(move(id)), name_(move(name)), role_(move(role)), password_(move(pw)) {}
    virtual ~User() = default;
    const string& id() const { return id_; }
    const string& name() const { return name_; }
    const string& role() const { return role_; }
    bool checkPassword(const string& p) const { return password_ == p; }
};

class Student : public User {
    int sem_; string dept_;
public:
    Student(string id, string name, string pw, int sem, string dept)
        : User(move(id), move(name), "Student", move(pw)), sem_(sem), dept_(move(dept)) {}
    int sem() const { return sem_; }
    const string& dept() const { return dept_; }
};
class Advisor : public User {
public:
    Advisor(string id, string name, string pw) : User(move(id), move(name), "Advisor", move(pw)) {}
};
class Accountant : public User {
public:
    Accountant(string id, string name, string pw) : User(move(id), move(name), "Accountant", move(pw)) {}
};
class HOD : public User {
public:
    HOD(string id, string name, string pw) : User(move(id), move(name), "HOD", move(pw)) {}
};
class Administrator : public User {
public:
    Administrator(string id, string name, string pw) : User(move(id), move(name), "Administrator", move(pw)) {}
};

// -------------------------------------------------------- RegistrationSystem
class RegistrationSystem {
public:
    static const Money FEE;
    vector<unique_ptr<User>> users;
    vector<Course> courses;
    vector<Registration> regs;
    vector<Notification> notes;
    map<string, Payment> payments;
    map<string, bool> holds, locked;
    bool open = true;
    int nextId = 1;

    RegistrationSystem() {
        users.push_back(make_unique<Student>("S101", "Aarav Shah", "1234", 3, "CSE"));
        users.push_back(make_unique<Student>("S102", "Diya Patel", "1234", 5, "CSE"));
        users.push_back(make_unique<Advisor>("A01", "Prof. Mehta", "1234"));
        users.push_back(make_unique<Accountant>("AC01", "Mr. Rao (Accountant)", "1234"));
        users.push_back(make_unique<HOD>("H01", "Dr. Iyer (HOD)", "1234"));
        users.push_back(make_unique<Administrator>("AD01", "Admin", "1234"));
        courses = {
            {"CS301", "Data Structures", "CSE", 3, 3, true},
            {"CS302", "Discrete Mathematics", "CSE", 3, 40, true},
            {"CS303", "Object Oriented Programming", "CSE", 3, 40, true},
            {"CS304", "Digital Logic", "CSE", 3, 40, false},
            {"CS501", "Operating Systems", "CSE", 5, 40, true},
            {"CS502", "Computer Networks", "CSE", 5, 40, true},
            {"CS503", "Database Systems", "CSE", 5, 40, true},
            {"CS201", "Computer Organization (Sem 2)", "CSE", 2, 30, true},
            {"CS401", "Algorithms (Sem 4)", "CSE", 4, 30, true}};
    }

    // ---- lookups
    Course* course(const string& code) {
        return findIf(courses, [&](const Course& c) { return c.code == code; });
    }
    User* userById(const string& id) {
        for (auto& u : users) if (u->id() == id) return u.get();
        return nullptr;
    }
    Registration* regById(int id) {
        return findIf(regs, [&](const Registration& r) { return r.id == id; });
    }
    vector<Registration*> regsOf(const string& sid, bool activeOnly = false) {
        vector<Registration*> out;
        for (auto& r : regs)
            if (r.sid == sid && (!activeOnly || r.status != Status::Rejected)) out.push_back(&r);
        return out;
    }

    // ---- overloaded course search (function overloading)
    vector<Course*> findCourses() {
        vector<Course*> v; for (auto& c : courses) v.push_back(&c); return v;
    }
    vector<Course*> findCourses(const string& text) {
        vector<Course*> v;
        string t = upper(text);
        for (auto& c : courses)
            if (upper(c.code + c.name).find(t) != string::npos) v.push_back(&c);
        return v;
    }
    vector<Course*> findCourses(int sem, const string& dept) {
        vector<Course*> v;
        for (auto& c : courses) if (c.sem == sem && c.dept == dept) v.push_back(&c);
        return v;
    }

    // ---- login
    User* login(const string& id, const string& pw) {
        User* u = userById(upper(id));
        return (u && u->checkPassword(pw)) ? u : nullptr;
    }

    // ---- notifications
    void notify(const string& uid, const string& text) { notes.insert(notes.begin(), {uid, text, now()}); }

    // ---- payment
    void pay(const Student& s, const string& method, const string& details) {
        Payment p; p.amount = FEE; p.method = method;
        try {
            p.ref = PaymentGateway::process(details);
        } catch (const RegistrationError&) {
            p.failed = true; payments[s.id()] = p; throw;    // record failure, rethrow to caller
        }
        p.paid = true; p.date = now();
        payments[s.id()] = p;
        notify("AC01", "Payment of " + FEE.str() + " from " + s.name() + " (" + s.id() + ") awaits verification.");
    }
    void verify(const string& sid, bool ok) {
        auto it = payments.find(sid);
        if (it == payments.end()) return;
        Payment& p = it->second;
        if (ok) { p.verified = true; notify(sid, "Payment verified. Registration is now unlocked."); }
        else { p.paid = false; p.verified = false; p.failed = true; notify(sid, "Payment verification failed. Please pay again."); }
    }

    // ---- registration
    void registerCourse(const Student& s, const string& rawCode) {
        string code = upper(rawCode);
        Course* c = course(code);
        if (!c) throw RegistrationError("No such course.");
        if (locked[s.id()]) throw RegistrationError("Registration is locked (slip generated).");
        if (!open) throw RegistrationError("Registration period is closed.");
        if (holds[s.id()]) throw RegistrationError("You have a registration hold. Contact the administrator.");
        auto p = payments.find(s.id());
        if (p == payments.end() || !p->second.paid) throw RegistrationError("Pay the semester fee first.");
        if (!p->second.verified) throw RegistrationError("Payment is awaiting accountant verification.");
        for (auto* r : regsOf(s.id(), true)) if (r->code == code) throw RegistrationError("Already registered for this course.");
        bool backlog = c->sem < s.sem();
        if (!backlog && !(c->sem == s.sem() && c->dept == s.dept() && c->listed))
            throw RegistrationError("You are not eligible for this course.");
        if (backlog && c->dept != s.dept()) throw RegistrationError("Not eligible for this backlog course.");
        if (c->seats <= 0) throw RegistrationError("Course is full.");
        c->seats--;
        regs.push_back({nextId++, s.id(), code, backlog, Status::Pending, Stage::Advisor, ""});
        notify("A01", s.name() + " submitted a registration for " + code + ".");
    }
    void drop(int rid, const string& sid) {
        Registration* r = regById(rid);
        if (!r || r->sid != sid) throw RegistrationError("No such registration.");
        if (locked[sid]) throw RegistrationError("Registration is locked.");
        if (r->status != Status::Rejected) course(r->code)->seats++;
        regs.erase(remove_if(regs.begin(), regs.end(), [&](const Registration& x) { return x.id == rid; }), regs.end());
    }
    // by = "Advisor" or "HOD"
    void decide(int rid, bool approve, const string& by, const string& reason = "") {
        Registration* r = regById(rid);
        if (!r || r->status != Status::Pending) throw RegistrationError("No such pending request.");
        if ((by == "Advisor") != (r->stage == Stage::Advisor)) throw RegistrationError("This request is not at your stage.");
        if (approve) {
            if (r->backlog && by == "Advisor") {
                r->stage = Stage::HOD;
                notify("H01", "Backlog registration " + r->code + " of " + r->sid + " needs HOD approval.");
                notify(r->sid, r->code + " forwarded to HOD for additional review.");
            } else {
                r->status = Status::Approved; r->stage = Stage::Done;
                notify(r->sid, r->code + " approved.");
            }
        } else {
            r->status = Status::Rejected; r->stage = Stage::Done;
            r->reason = reason.empty() ? "No reason given" : reason;
            course(r->code)->seats++;
            notify(r->sid, r->code + " rejected: " + r->reason);
        }
    }
    bool slipReady(const string& sid) {
        auto a = regsOf(sid, true);
        return !a.empty() && all_of(a.begin(), a.end(), [](Registration* r) { return r->status == Status::Approved; });
    }
    void lock(const string& sid) {
        if (!slipReady(sid)) throw RegistrationError("All courses must be approved first.");
        locked[sid] = true;
        notify(sid, "Registration locked. Slip generated.");
    }
};
const Money RegistrationSystem::FEE(45000);


// ============================ WEB SERVER ============================

#include <arpa/inet.h>
#include <csignal>
#include <fstream>
#include <netinet/in.h>
#include <cstring>
#include <random>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

// ----------------------------------------------------------------- state
RegistrationSystem sys;
struct Session { string uid, flashClass, flashText; };
map<string, Session> sessions;           // cookie token -> session
string g_tab;                            // tab of the page being drawn / acted on

struct Req {
    string method, path;
    map<string, string> q, f, cookies;   // query, form body, cookies
};
struct Res {
    int code = 200;
    string type = "text/html; charset=utf-8", body;
    vector<string> headers;
};

// --------------------------------------------------------------- utilities
static string esc(const string& s) {
    string o;
    for (char c : s) switch (c) {
        case '&': o += "&amp;"; break;
        case '<': o += "&lt;"; break;
        case '>': o += "&gt;"; break;
        case '"': o += "&quot;"; break;
        default: o += c;
    }
    return o;
}
static string urlEnc(const string& s) {
    static const char* hex = "0123456789ABCDEF";
    string o;
    for (unsigned char c : s) {
        if (isalnum(c)) o += c;
        else { o += '%'; o += hex[c >> 4]; o += hex[c & 15]; }
    }
    return o;
}
static string urlDec(const string& s) {
    string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') o += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && isxdigit((unsigned char)s[i + 1]) && isxdigit((unsigned char)s[i + 2])) {
            o += (char)stoi(s.substr(i + 1, 2), nullptr, 16); i += 2;
        } else o += s[i];
    }
    return o;
}
static map<string, string> parseForm(const string& s) {
    map<string, string> m;
    stringstream ss(s); string kv;
    while (getline(ss, kv, '&')) {
        auto eq = kv.find('=');
        if (eq == string::npos) m[urlDec(kv)] = "";
        else m[urlDec(kv.substr(0, eq))] = urlDec(kv.substr(eq + 1));
    }
    return m;
}
static string trim(string s) {
    size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
    return a == string::npos ? "" : s.substr(a, b - a + 1);
}
static string newToken() {
    static mt19937_64 rng(random_device{}());
    stringstream ss; ss << hex << rng() << rng();
    return ss.str();
}

static Res redirect(const string& to, const string& cookie = "") {
    Res r; r.code = 303; r.headers.push_back("Location: " + to);
    if (!cookie.empty()) r.headers.push_back("Set-Cookie: " + cookie);
    return r;
}
static Res page(const string& body) { Res r; r.body = body; return r; }

static int dayOfMonth() { time_t t = time(nullptr); return localtime(&t)->tm_mday; }

// --------------------------------------------------------------- HTML bits
static const char* QUOTES[] = {
    "Education is the passport to the future.",
    "The beautiful thing about learning is that no one can take it away from you.",
    "Small steps every day lead to big results.",
    "An investment in knowledge pays the best interest."};

static string greet(const string& role) {
    if (role == "Student") return "Plan your semester and register for your courses.";
    if (role == "Advisor") return "Review and guide your students' course choices.";
    if (role == "Accountant") return "Keep every fee record accurate.";
    if (role == "HOD") return "Finalize the semester subjects and review backlog requests.";
    return "Manage courses, holds and the registration period.";
}
static vector<string> tabsFor(const string& role) {
    if (role == "Student") return {"Courses", "My Registrations", "Payment", "Slip", "Notifications"};
    if (role == "Advisor") return {"Pending Approvals", "Notifications"};
    if (role == "HOD") return {"Backlog Approvals", "Subject List", "Notifications"};
    if (role == "Accountant") return {"Verify Payments", "Notifications"};
    return {"Manage Courses", "Settings", "Notifications"};
}
static string badge(const string& s) {
    string c = (s == "approved" || s == "paid") ? "ok" : s == "pending" ? "wa" : (s == "rejected" || s == "failed") ? "er" : "";
    return "<span class=\"b " + c + "\">" + s + "</span>";
}
// A small POST form with one button (replaces JavaScript onclick handlers).
static string btn(const string& action, const vector<pair<string, string>>& hid, const string& label,
                  const string& cls = "s", bool disabled = false) {
    string h = "<form method=\"post\" action=\"" + action + "\"><input type=\"hidden\" name=\"tab\" value=\"" + esc(g_tab) + "\">";
    for (auto& [k, v] : hid) h += "<input type=\"hidden\" name=\"" + k + "\" value=\"" + esc(v) + "\">";
    return h + "<button class=\"" + cls + "\"" + (disabled ? " disabled" : "") + ">" + label + "</button></form>";
}
static string card(const string& inner) { return "<div class=\"card\">" + inner + "</div>"; }
static string rowsOrEmpty(const string& rows, int cols, const string& msg) {
    return rows.empty() ? "<tr><td colspan=" + to_string(cols) + ">" + msg + "</td></tr>" : rows;
}

static string readFile(const string& name) {
    ifstream in(name, ios::binary);
    if (!in) return "";
    stringstream ss; ss << in.rdbuf(); return ss.str();
}
// Replaces every {{KEY}} in an HTML template with its value.
static string fill(string tpl, const map<string, string>& vals) {
    for (auto& [k, v] : vals) {
        string key = "{{" + k + "}}";
        for (size_t p = 0; (p = tpl.find(key, p)) != string::npos; p += v.size()) tpl.replace(p, key.size(), v);
    }
    return tpl;
}

static string layout(const User* u, const string& content, const Session* ss) {
    string flash;
    if (ss && !ss->flashText.empty()) flash = "<div class=\"msg " + ss->flashClass + "\">" + esc(ss->flashText) + "</div>";
    string who = u ? esc(u->name()) + " (" + u->role() + ") <form method=\"post\" action=\"/logout\"><button class=\"g s\">Logout</button></form>" : "";
    string tpl = readFile("index.html");
    if (tpl.empty()) return "index.html not found - run the server from the project folder.";
    return fill(tpl, {{"BODY_CLASS", u ? "in" : "out"}, {"WHO", who}, {"FLASH", flash}, {"CONTENT", content}});
}

// -------------------------------------------------------------- login page
static Res loginPage(bool err) {
    string tpl = readFile("login.html");
    if (tpl.empty()) return page("login.html not found - run the server from the project folder.");
    string e = err ? "<div class=\"msg er\" style=\"margin-top:10px\">Invalid ID or password.</div>" : "";
    return page(layout(nullptr, fill(tpl, {{"ERROR", e}}), nullptr));
}

// -------------------------------------------------------------- tab views
static string stepper(const string& sid) {
    auto p = sys.payments.find(sid);
    int st = 0;
    if (p != sys.payments.end() && p->second.paid) st = 1;
    if (p != sys.payments.end() && p->second.verified) st = 2;
    if (!sys.regsOf(sid, true).empty()) st = 3;
    if (sys.slipReady(sid)) st = 4;
    if (sys.locked[sid]) st = 5;
    const char* names[] = {"💳 Pay Fee", "✅ Verification", "📝 Register", "👨‍🏫 Approval", "🧾 Slip"};
    string h = "<div class=\"steps\">";
    for (int i = 0; i < 5; i++)
        h += "<div class=\"step " + string(i < st ? "done" : i == st ? "now" : "") + "\">" + names[i] + "</div>";
    return h + "</div>";
}

static string coursesView(Student& s, Req& req) {
    string q = trim(req.q["q"]);
    auto list = q.empty() ? sys.findCourses() : sys.findCourses(q);          // overloaded search
    string rows;
    for (auto* c : list) {
        if (!(c->dept == s.dept() && ((c->sem == s.sem() && c->listed) || c->sem < s.sem()))) continue;
        bool bl = c->sem < s.sem();
        rows += "<tr><td>" + c->code + "</td><td>" + esc(c->name) + "</td><td>" + to_string(c->sem) + "</td><td>" +
                (bl ? "<span class=\"b\">Backlog</span>" : "Regular") + "</td><td>" + to_string(c->seats) + "</td><td>" +
                btn("/register", {{"code", c->code}}, bl ? "Register Backlog" : "Register") + "</td></tr>";
    }
    auto p = sys.payments.find(s.id());
    string info = (p == sys.payments.end() || !p->second.paid) ? "Pay the semester fee to unlock registration."
                  : !p->second.verified ? "Payment received – awaiting accountant verification." : "Registration unlocked.";
    return card("<h2>📚 Available Courses – Semester " + to_string(s.sem()) + "</h2><p class=\"mu\">" +
                string(sys.open ? "Registration period is open." : "Registration period is CLOSED.") + " " + info + "</p>"
                "<div class=\"row\"><form method=\"get\" action=\"/home\"><input type=\"hidden\" name=\"tab\" value=\"Courses\">"
                "<input name=\"q\" placeholder=\"Search course…\" value=\"" + esc(q) + "\"> <button class=\"g\">Search</button></form></div>"
                "<table><tr><th>Code</th><th>Course</th><th>Sem</th><th>Type</th><th>Seats</th><th></th></tr>" +
                rowsOrEmpty(rows, 6, "No courses found.") + "</table>");
}

static string myRegsView(Student& s) {
    string rows;
    for (auto* r : sys.regsOf(s.id()))
        rows += "<tr><td>" + r->code + "</td><td>" + esc(sys.course(r->code)->name) + "</td><td>" + (r->backlog ? "Backlog" : "Regular") +
                "</td><td>" + badge(toStr(r->status)) + (r->stage == Stage::HOD && r->status == Status::Pending ? " <span class=\"mu\">(with HOD)</span>" : "") +
                (r->reason.empty() ? "" : "<div class=\"mu\">Reason: " + esc(r->reason) + "</div>") + "</td><td>" +
                btn("/drop", {{"rid", to_string(r->id)}}, "Drop", "d s") + "</td></tr>";
    return card("<h2>📝 Registration Status</h2><table><tr><th>Code</th><th>Course</th><th>Type</th><th>Status</th><th></th></tr>" +
                rowsOrEmpty(rows, 5, "No registrations yet.") + "</table>" +
                (sys.locked[s.id()] ? "<p><span class=\"b ok\">Locked</span> Registration finalised.</p>" : ""));
}

static string paymentView(Student& s) {
    auto it = sys.payments.find(s.id());
    string status = "<span class=\"b\">not paid</span>";
    bool paid = false;
    if (it != sys.payments.end()) {
        auto& p = it->second; paid = p.paid;
        string st = p.paid ? "paid" : p.failed ? "failed" : "pending";
        status = badge(st) + (p.verified ? " <span class=\"b ok\">verified</span>" : "") +
                 (p.ref.empty() ? "" : " <span class=\"mu\">Ref " + p.ref + " " + p.date + "</span>");
    }
    string form;
    if (!paid)
        form = "<form method=\"post\" action=\"/pay\" class=\"row\"><input type=\"hidden\" name=\"tab\" value=\"Payment\">"
               "<select name=\"pm\"><option>Card</option><option>UPI</option><option>Net Banking</option></select> "
               "<input name=\"pd\" placeholder=\"Card / UPI / account number\"> <button>Pay " + RegistrationSystem::FEE.str() +
               "</button> <button class=\"g\" formaction=\"/paycancel\">Cancel</button></form><p class=\"mu\">Demo: any 4+ characters succeed; details ending in 0000 are declined.</p>";
    return card("<h2>💳 Semester Fee Payment</h2><p>Amount due: <b>" + RegistrationSystem::FEE.str() + "</b></p><p>Status: " + status + "</p>" + form);
}

static string slipView(Student& s) {
    if (sys.locked[s.id()]) {
        string rows;
        for (auto* r : sys.regsOf(s.id(), true))
            rows += "<tr><td>" + r->code + "</td><td>" + esc(sys.course(r->code)->name) + "</td><td>" + (r->backlog ? "Backlog" : "Regular") + "</td></tr>";
        return card("<div class=\"slip\"><h2>🎓 Registration Slip</h2><p><b>" + esc(s.name()) + "</b> (" + s.id() + ") – " + s.dept() +
                    ", Semester " + to_string(s.sem()) + "</p><table><tr><th>Code</th><th>Course</th><th>Type</th></tr>" + rows +
                    "</table><p>Fee paid: " + RegistrationSystem::FEE.str() + " · Status: <b>Registration Confirmed</b></p></div>"
                    "<p class=\"noprint mu\">To print the slip, use your browser's print option (Ctrl+P).</p>");
    }
    bool ready = sys.slipReady(s.id());
    return card("<h2>🧾 Registration Slip</h2><p>" + string(ready ? "All courses are approved. You can lock registration and generate your slip."
                : "Slip becomes available once all your registered courses are approved.") + "</p>" +
                btn("/lock", {}, "Lock Registration &amp; Generate Slip", "", !ready));
}

static string notesView(User& u) {
    string rows;
    for (auto& n : sys.notes) if (n.uid == u.id()) rows += "<tr><td class=\"mu\">" + n.time + "</td><td>" + esc(n.text) + "</td></tr>";
    return card("<h2>🔔 Notifications</h2><table>" + rowsOrEmpty(rows, 2, "No notifications.") + "</table>");
}

static string approvalView(Stage stage, const string& by) {
    string rows;
    for (auto& r : sys.regs) {
        if (r.status != Status::Pending || r.stage != stage) continue;
        User* s = sys.userById(r.sid);
        auto p = sys.payments.find(r.sid);
        string rid = to_string(r.id);
        rows += "<tr><td>" + esc(s->name()) + " (" + s->id() + ")</td><td>" + r.code + " – " + esc(sys.course(r.code)->name) + "</td><td>" +
                (r.backlog ? "Backlog" : "Regular") + "</td><td>" + ((p != sys.payments.end() && p->second.verified) ? "Verified" : "—") + "</td><td>" +
                btn("/decide", {{"rid", rid}, {"act", "approve"}}, "Approve", "k s") +
                " <form method=\"get\" action=\"/reject\"><input type=\"hidden\" name=\"tab\" value=\"" + esc(g_tab) + "\"><input type=\"hidden\" name=\"rid\" value=\"" + rid + "\"><button class=\"d s\">Reject</button></form></td></tr>";
    }
    return card("<h2>" + string(by == "HOD" ? "Backlog registrations awaiting HOD approval" : "👨‍🏫 Pending registration requests") + "</h2>"
                "<table><tr><th>Student</th><th>Course</th><th>Type</th><th>Payment</th><th></th></tr>" + rowsOrEmpty(rows, 5, "No pending requests.") + "</table>" +
                (by == "Advisor" ? "<p class=\"mu\">Backlog registrations are forwarded to the HOD after your approval.</p>" : ""));
}

static string subjectListView() {
    string rows;
    for (auto& c : sys.courses)
        rows += "<tr><td>" + c.code + "</td><td>" + esc(c.name) + "</td><td>" + to_string(c.sem) + "</td><td><span class=\"b " + (c.listed ? "ok" : "wa") + "\">" +
                (c.listed ? "listed" : "not listed") + "</span></td><td>" + btn("/togglelist", {{"code", c.code}}, c.listed ? "Unlist" : "List", "s g") + "</td></tr>";
    return card("<h2>📋 Finalize Subject List for Semester</h2><table><tr><th>Code</th><th>Course</th><th>Sem</th><th>Status</th><th></th></tr>" + rows + "</table>");
}

static string verifyView() {
    string rows;
    for (auto& [sid, p] : sys.payments)
        if (p.paid && !p.verified)
            rows += "<tr><td>" + sid + "</td><td>" + p.amount.str() + "</td><td>" + esc(p.method) + "</td><td>" + p.ref + "</td><td>" +
                    btn("/verify", {{"sid", sid}, {"ok", "1"}}, "Verify", "k s") + " " + btn("/verify", {{"sid", sid}, {"ok", "0"}}, "Reject", "d s") + "</td></tr>";
    return card("<h2>🧮 Payments Awaiting Verification</h2><table><tr><th>Student</th><th>Amount</th><th>Method</th><th>Ref</th><th></th></tr>" +
                rowsOrEmpty(rows, 5, "Nothing to verify.") + "</table>");
}

static string manageCoursesView() {
    string rows;
    for (auto& c : sys.courses)
        rows += "<tr><td>" + c.code + "</td><td>" + esc(c.name) + "</td><td>" + c.dept + "</td><td>" + to_string(c.sem) + "</td><td>"
                "<form method=\"post\" action=\"/setseats\"><input type=\"hidden\" name=\"tab\" value=\"Manage Courses\"><input type=\"hidden\" name=\"code\" value=\"" + c.code +
                "\"><input type=\"number\" name=\"seats\" min=\"0\" value=\"" + to_string(c.seats) + "\" style=\"width:70px\"> <button class=\"s g\">Save</button></form></td><td>" +
                btn("/delcourse", {{"code", c.code}}, "Delete", "d s") + "</td></tr>";
    return card("<h2>🏫 Manage Courses</h2><table><tr><th>Code</th><th>Name</th><th>Dept</th><th>Sem</th><th>Seats</th><th></th></tr>" + rows + "</table>"
                "<h2 style=\"margin-top:18px\">Add Course</h2><form method=\"post\" action=\"/addcourse\" class=\"row\"><input type=\"hidden\" name=\"tab\" value=\"Manage Courses\">"
                "<input name=\"code\" placeholder=\"Code\" size=7> <input name=\"name\" placeholder=\"Name\"> <input name=\"sem\" type=\"number\" placeholder=\"Sem\" style=\"width:70px\"> "
                "<input name=\"seats\" type=\"number\" placeholder=\"Seats\" style=\"width:80px\"> <button>Add</button></form>");
}

static string settingsView() {
    string rows;
    for (auto& u : sys.users)
        if (u->role() == "Student") {
            bool h = sys.holds[u->id()];
            rows += "<tr><td>" + u->id() + "</td><td>" + u->name() + "</td><td>" + (h ? "<span class=\"b er\">Hold</span>" : "<span class=\"b ok\">Clear</span>") +
                    "</td><td>" + btn("/togglehold", {{"sid", u->id()}}, h ? "Remove hold" : "Place hold", "s g") + "</td></tr>";
        }
    return card("<h2>🗓️ Registration Period</h2><p>Currently: " + string(sys.open ? "<span class=\"b ok\">Open</span>" : "<span class=\"b er\">Closed</span>") + "</p>" +
                btn("/toggleopen", {}, string(sys.open ? "Close" : "Open") + " registration", "")) +
           card("<h2>Student Holds</h2><table><tr><th>ID</th><th>Name</th><th>Status</th><th></th></tr>" + rows + "</table>");
}

// -------------------------------------------------------------- home page
static Res homePage(User* u, Session& ss, Req& req) {
    auto tabs = tabsFor(u->role());
    string tab = req.q.count("tab") ? req.q["tab"] : "";
    if (find(tabs.begin(), tabs.end(), tab) == tabs.end()) tab = tabs[0];
    g_tab = tab;

    string content = "<div class=\"hero\"><h2>Welcome, " + esc(u->name()) + " 👋</h2><p>" + greet(u->role()) + " — “" + QUOTES[dayOfMonth() % 4] + "”</p></div>";
    if (u->role() == "Student") content += stepper(u->id());
    content += "<div class=\"tabs\">";
    for (auto& t : tabs) content += "<a class=\"tab" + string(t == tab ? " on" : "") + "\" href=\"/home?tab=" + urlEnc(t) + "\">" + t + "</a>";
    content += "</div>";

    Student* st = dynamic_cast<Student*>(u);             // polymorphic user -> concrete type
    if (tab == "Courses") content += coursesView(*st, req);
    else if (tab == "My Registrations") content += myRegsView(*st);
    else if (tab == "Payment") content += paymentView(*st);
    else if (tab == "Slip") content += slipView(*st);
    else if (tab == "Pending Approvals") content += approvalView(Stage::Advisor, "Advisor");
    else if (tab == "Backlog Approvals") content += approvalView(Stage::HOD, "HOD");
    else if (tab == "Subject List") content += subjectListView();
    else if (tab == "Verify Payments") content += verifyView();
    else if (tab == "Manage Courses") content += manageCoursesView();
    else if (tab == "Settings") content += settingsView();
    else content += notesView(*u);

    Res r = page(layout(u, content, &ss));
    ss.flashText.clear();                                // flash messages show once
    return r;
}

// ------------------------------------------------------------ POST actions
static void flash(Session& ss, bool ok, const string& text) { ss.flashClass = ok ? "ok" : "er"; ss.flashText = text; }

template <typename F>
static void attempt(Session& ss, F action, const string& okMsg) {
    try { action(); if (!okMsg.empty()) flash(ss, true, okMsg); }
    catch (const RegistrationError& e) { flash(ss, false, e.what()); }
}

static Res doAction(User* u, Session& ss, Req& req) {
    const string& p = req.path;
    auto& f = req.f;
    string role = u->role();
    auto need = [&](const string& r) { return role == r; };
    Student* st = dynamic_cast<Student*>(u);
    string tab = f.count("tab") ? f["tab"] : "";
    string back = "/home?tab=" + urlEnc(tab);

    if (p == "/pay" && st)
        attempt(ss, [&] { sys.pay(*st, f["pm"], trim(f["pd"])); }, "Payment successful. Awaiting accountant verification.");
    else if (p == "/paycancel" && st)
        flash(ss, false, "Payment cancelled.");
    else if (p == "/register" && st)
        attempt(ss, [&] { sys.registerCourse(*st, f["code"]); }, "Registration submitted for advisor approval.");
    else if (p == "/drop" && st)
        attempt(ss, [&] { sys.drop(atoi(f["rid"].c_str()), st->id()); }, "Course dropped.");
    else if (p == "/lock" && st)
        attempt(ss, [&] { sys.lock(st->id()); }, "Slip generated.");
    else if (p == "/decide" && (need("Advisor") || need("HOD"))) {
        bool approve = f["act"] == "approve";
        attempt(ss, [&] { sys.decide(atoi(f["rid"].c_str()), approve, role, trim(f["reason"])); }, "");
    } else if (p == "/verify" && need("Accountant")) {
        sys.verify(f["sid"], f["ok"] == "1");
    } else if (p == "/togglelist" && need("HOD")) {
        if (Course* c = sys.course(f["code"])) c->listed = !c->listed;
    } else if (p == "/addcourse" && need("Administrator")) {
        string code = upper(trim(f["code"])), name = trim(f["name"]);
        int sem = atoi(f["sem"].c_str()), seats = atoi(f["seats"].c_str());
        if (code.empty() || name.empty() || sem <= 0 || seats <= 0) flash(ss, false, "Fill all fields.");
        else if (sys.course(code)) flash(ss, false, "Course code already exists.");
        else { sys.courses.push_back({code, name, "CSE", sem, seats, true}); flash(ss, true, "Course added."); }
    } else if (p == "/delcourse" && need("Administrator")) {
        string code = f["code"];
        sys.courses.erase(remove_if(sys.courses.begin(), sys.courses.end(), [&](const Course& c) { return c.code == code; }), sys.courses.end());
    } else if (p == "/setseats" && need("Administrator")) {
        Course* c = sys.course(f["code"]); int seats = atoi(f["seats"].c_str());
        if (c && seats >= 0) c->seats = seats; else flash(ss, false, "Invalid seat count.");
    } else if (p == "/toggleopen" && need("Administrator")) {
        sys.open = !sys.open;
    } else if (p == "/togglehold" && need("Administrator")) {
        User* s = sys.userById(f["sid"]);
        if (s && s->role() == "Student") sys.holds[s->id()] = !sys.holds[s->id()];
    } else flash(ss, false, "That action is not allowed for your role.");
    return redirect(back);
}

// --------------------------------------------------------- reject-reason page
// Replaces the JavaScript prompt('Reason for rejection:') of the original.
static Res rejectPage(User* u, Req& req) {
    string tab = req.q["tab"], rid = req.q["rid"];
    string c = card("<h2>Reject request</h2><form method=\"post\" action=\"/decide\" style=\"display:block\">"
                    "<input type=\"hidden\" name=\"tab\" value=\"" + esc(tab) + "\"><input type=\"hidden\" name=\"rid\" value=\"" + esc(rid) +
                    "\"><input type=\"hidden\" name=\"act\" value=\"reject\"><p>Reason for rejection:</p>"
                    "<input name=\"reason\" style=\"width:100%;margin-bottom:10px\" autofocus> <button class=\"d\">OK</button> "
                    "<a class=\"tab\" href=\"/home?tab=" + urlEnc(tab) + "\">Cancel</a></form>");
    return page(layout(u, c, nullptr));
}

// ------------------------------------------------- saving (replaces localStorage)
static const char* DATA_FILE = "data.txt";
static string escF(const string& s) {
    string o;
    for (char c : s) { if (c == '\\') o += "\\\\"; else if (c == '\t') o += "\\t"; else if (c == '\n') o += "\\n"; else o += c; }
    return o;
}
static string unescF(const string& s) {
    string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) { char n = s[++i]; o += n == 't' ? '\t' : n == 'n' ? '\n' : n; }
        else o += s[i];
    }
    return o;
}
static void saveData() {
    ofstream out(DATA_FILE, ios::trunc);
    if (!out) return;
    auto line = [&](initializer_list<string> fields) {
        bool first = true;
        for (auto& x : fields) { if (!first) out << '\t'; out << escF(x); first = false; }
        out << '\n';
    };
    line({"O", to_string(sys.open), to_string(sys.nextId)});
    for (auto& [k, v] : sys.holds) line({"H", k, to_string(v)});
    for (auto& [k, v] : sys.locked) line({"L", k, to_string(v)});
    for (auto& [sid, p] : sys.payments)
        line({"P", sid, to_string(p.amount.value()), p.method, p.ref, p.date, to_string(p.paid), to_string(p.verified), to_string(p.failed)});
    for (auto& r : sys.regs)
        line({"R", to_string(r.id), r.sid, r.code, to_string(r.backlog), to_string((int)r.status), to_string((int)r.stage), r.reason});
    for (auto it = sys.notes.rbegin(); it != sys.notes.rend(); ++it) line({"N", it->uid, it->text, it->time});
    for (auto& c : sys.courses) line({"C", c.code, c.name, c.dept, to_string(c.sem), to_string(c.seats), to_string(c.listed)});
}
static void loadData() {
    ifstream in(DATA_FILE);
    if (!in) return;
    RegistrationSystem fresh;
    fresh.courses.clear();
    string ln;
    while (getline(in, ln)) {
        vector<string> f; stringstream ss(ln); string x;
        while (getline(ss, x, '\t')) f.push_back(unescF(x));
        if (f.empty()) continue;
        f.resize(max<size_t>(f.size(), 9));              // trailing empty fields are not split off
        const string& t = f[0];
        if (t == "O") { fresh.open = f[1] == "1"; fresh.nextId = atoi(f[2].c_str()); }
        else if (t == "H") fresh.holds[f[1]] = f[2] == "1";
        else if (t == "L") fresh.locked[f[1]] = f[2] == "1";
        else if (t == "P") {
            Payment p; p.amount = Money(atol(f[2].c_str())); p.method = f[3]; p.ref = f[4]; p.date = f[5];
            p.paid = f[6] == "1"; p.verified = f[7] == "1"; p.failed = f[8] == "1"; fresh.payments[f[1]] = p;
        } else if (t == "R") {
            Registration r{atoi(f[1].c_str()), f[2], f[3], f[4] == "1", (Status)atoi(f[5].c_str()), (Stage)atoi(f[6].c_str()), f[7]};
            fresh.regs.push_back(r);
        } else if (t == "N") fresh.notes.insert(fresh.notes.begin(), {f[1], f[2], f[3]});
        else if (t == "C") fresh.courses.push_back({f[1], f[2], f[3], atoi(f[4].c_str()), atoi(f[5].c_str()), f[6] == "1"});
    }
    sys = move(fresh);
}

// ---------------------------------------------------------------- routing
static Res handle(Req& req) {
    if (req.method == "GET" && req.path == "/style.css") {
        ifstream in("style.css", ios::binary);
        Res r; r.type = "text/css; charset=utf-8";
        if (!in) { r.code = 404; r.body = "style.css not found (run the server from its folder)"; return r; }
        stringstream ss; ss << in.rdbuf(); r.body = ss.str(); return r;
    }
    Session* ss = nullptr; User* u = nullptr;
    if (req.cookies.count("sid")) {
        auto it = sessions.find(req.cookies["sid"]);
        if (it != sessions.end()) { ss = &it->second; u = sys.userById(ss->uid); }
    }
    if (req.method == "POST" && req.path == "/login") {
        User* user = sys.login(trim(req.f["uid"]), req.f["pw"]);
        if (!user) return redirect("/?err=1");
        string tok = newToken(); sessions[tok] = {user->id(), "", ""};
        return redirect("/home", "sid=" + tok + "; Path=/; HttpOnly; SameSite=Lax");
    }
    if (req.method == "GET" && req.path == "/confirm-reset")
        return page(layout(nullptr, card("<h2>Reset demo data</h2><p>Reset all data?</p>"
                    "<form method=\"post\" action=\"/reset\"><button class=\"d\">OK</button></form> <a class=\"tab\" href=\"/\">Cancel</a>"), nullptr));
    if (req.method == "POST" && req.path == "/reset") {
        sys = RegistrationSystem(); sessions.clear(); saveData();
        return redirect("/", "sid=; Path=/; Max-Age=0");
    }
    if (req.method == "POST" && req.path == "/logout") {
        if (req.cookies.count("sid")) sessions.erase(req.cookies["sid"]);
        return redirect("/", "sid=; Path=/; Max-Age=0");
    }
    if (req.method == "GET" && req.path == "/") return u ? redirect("/home") : loginPage(req.q.count("err"));
    if (!u) return redirect("/");                                   // everything below needs login
    if (req.method == "GET" && req.path == "/home") return homePage(u, *ss, req);
    if (req.method == "GET" && req.path == "/reject" && (u->role() == "Advisor" || u->role() == "HOD")) return rejectPage(u, req);
    if (req.method == "POST") { Res r = doAction(u, *ss, req); saveData(); return r; }
    Res r; r.code = 404; r.body = "Not found"; return r;
}

// ------------------------------------------------------------ HTTP plumbing
static bool readRequest(int fd, string& raw) {
    char buf[4096];
    while (true) {
        size_t pos = raw.find("\r\n\r\n");
        if (pos != string::npos) {
            string head = raw.substr(0, pos);
            for (auto& c : head) c = tolower((unsigned char)c);
            size_t cl = 0, k = head.find("content-length:");
            if (k != string::npos) cl = (size_t)atol(head.c_str() + k + 15);
            if (raw.size() >= pos + 4 + cl) return true;
        }
        ssize_t n = recv(fd, buf, sizeof buf, 0);
        if (n <= 0 || raw.size() > (1u << 20)) return false;
        raw.append(buf, (size_t)n);
    }
}
static Req parseRequest(const string& raw) {
    Req r;
    size_t hend = raw.find("\r\n\r\n");
    string head = raw.substr(0, hend), body = raw.substr(hend + 4);
    stringstream hs(head); string line;
    getline(hs, line);
    stringstream rl(line); string target; rl >> r.method >> target;
    auto qm = target.find('?');
    r.path = target.substr(0, qm);
    if (qm != string::npos) r.q = parseForm(target.substr(qm + 1));
    while (getline(hs, line)) {
        line = trim(line);
        if (line.size() > 7 && strncasecmp(line.c_str(), "Cookie:", 7) == 0) {
            stringstream cs(line.substr(7)); string kv;
            while (getline(cs, kv, ';')) {
                kv = trim(kv); auto eq = kv.find('=');
                if (eq != string::npos) r.cookies[kv.substr(0, eq)] = kv.substr(eq + 1);
            }
        }
    }
    if (r.method == "POST") r.f = parseForm(body);
    return r;
}
static void sendResponse(int fd, const Res& r) {
    string status = r.code == 200 ? "OK" : r.code == 303 ? "See Other" : r.code == 404 ? "Not Found" : "Error";
    string out = "HTTP/1.1 " + to_string(r.code) + " " + status + "\r\nContent-Type: " + r.type +
                 "\r\nContent-Length: " + to_string(r.body.size()) + "\r\nConnection: close\r\n";
    for (auto& h : r.headers) out += h + "\r\n";
    out += "\r\n" + r.body;
    size_t sent = 0;
    while (sent < out.size()) {
        ssize_t n = send(fd, out.data() + sent, out.size() - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

int main(int argc, char** argv) {
    int port = argc > 1 ? atoi(argv[1]) : 8080;
    loadData();
    signal(SIGPIPE, SIG_IGN);
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1; setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY); addr.sin_port = htons((uint16_t)port);
    if (bind(srv, (sockaddr*)&addr, sizeof addr) < 0 || listen(srv, 16) < 0) { perror("bind/listen"); return 1; }
    cout << "Course registration server running at http://localhost:" << port << "  (Ctrl+C to stop)\n";
    while (true) {
        int fd = accept(srv, nullptr, nullptr);
        if (fd < 0) continue;
        timeval tv{5, 0}; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
        string raw;
        if (readRequest(fd, raw)) {
            try { Req req = parseRequest(raw); sendResponse(fd, handle(req)); }
            catch (const exception& e) { Res r; r.code = 500; r.body = e.what(); sendResponse(fd, r); }
        }
        close(fd);
    }
}