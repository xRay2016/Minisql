create table Students
(
SNO string primary key,
SNAME string,
SEX string,
BDATE string,
HEIGHT double,
DEPARTMENT string
);
create table Courses
(
CNO string primary key,
CNAME string,
LHOUR string,
CREDIT double,
SEMESTER string
);
create table SC
(
SNO string,
CNO string,
GRADE int,
);