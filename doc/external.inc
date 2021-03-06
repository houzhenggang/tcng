%
% external.inc - Definition of tcng "external" protocol
%
% Written 2001,2002,2004 by Werner Almesberger
% Copyright 2001 Network Robots
% Copyright 2002 Bivio Networks
% Copyright 2004 Werner Almesberger
%


The ``external'' interface of \prog{tcng} allows the addition of support
for hardware
accelerators and other traffic control implementations not directly known to
\prog{tcng}, without the need to understand and change \prog{tcng} itself.

\prog{tcng} attempts to use external programs for all traffic control elements
(i.e. classifiers, queuing disciplines, or combinations thereof) for which
such an external program is specified with the \raw{-x} command line option.
\raw{-x} is used as follows:

\raw{-x \meta{element}:\meta{external\_target}}

\begin{description}
  \item[\meta{element}] is the name of the element. Currently, only
    the names \raw{if} and \raw{all} are recognized. All other names are
    treated as if \raw{if} had been specified.
    \raw{-x all:\meta{external\_target}} has the same effect as the
    external program returning \name{all} in the configuration query,
    as described in section \ref{cfgqout}.
  \item[\meta{external\_target}] is the name of the external target. This
    name identifies the (external) program \prog{tcng} will invoke for
    configuration.
\end{description}

\prog{tcng} invokes the external program to probe whether it is able to
process the
configuration. If it is, \prog{tcng} may invoke it a second time to request
generation of the components that implement the desired configuration.

Note: \prog{tcng} currently omits the probe and always tries to implement the
configuration.


%------------------------------------------------------------------------------


\section{Program invocation}

\prog{tcng} invokes the external program with a keyword indicating
the desired operation, followed by more arguments specific to the
operation, and optional user arguments.

The following operations are possible:
\begin{description}
  \item[\raw{config}] obtain general configuration parameters for the
    external target.
  \item[\raw{probe}] verify whether the requested configuration can be
    supported by the external target.
  \item[\raw{build}] generate all components necessary to implement the
    configuration. If possible, the external program should not
    implement parts of the configuration during the build run, i.e. if
    the external program needs to configure a hardware accelerator, it
    should generate the kernel modules needed to set up the hardware,
    and not manipulate the hardware by itself.
\end{description}

The program must have the name \raw{tcc-ext-\meta{external\_target}},
and \prog{tcng} must be able to find it in the \raw{PATH}.

User arguments are passed on the \prog{tcng} or \prog{tcsim} command line
with the
\verb"-X" option for phase \name{x}, see sections \ref{tccusg} and \ref{phtcc}.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Configuration query}

The configuration query uses only the keyword \raw{config} and the
user-provided arguments. Typically, the configuration query simply
returns a list of hard-coded key phrases, possibly followed by some or
all of the user arguments. The output of the configuration query is
described in section \ref{cfgqout}.

For the configuration query, \prog{tcng} does not send any data to
the standard input of the external program.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Capability probe}

The capability probe is currently not used.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Build run}

In the build run, the keyword \raw{build} is followed by
a unique name that can be used to identify objects generated by the
external program.

The unique name is a string of alphanumeric characters (e.g. \verb"f200"),
which can be used as a name for
objects (files, traffic control elements, etc.) generated by the
external program. The name is generated such that it is unique for
this run of \prog{tcng}. Consecutive runs of \prog{tcng} may use the same name.
The name is short enough that it is still a valid traffic control
element identifier if the external program adds two more characters.
Note that the unique name may begin with a digit, so at least one
letter must be prepended in order to obtain a valid C identifier.

The external program is under no obligation of using the identifier
provided by \prog{tcng}. At the end of a build run, it returns a list of
elements for the traffic control subsystem of the kernel
that need to be loaded and initialized to implement the
configuration, as described in section \ref{buildrun}.

\begin{figure}[ht]
\begin{center}
\epsfig{file=build.eps}
\end{center}
\caption{Module usage at external interface in build run.}
\label{buildfig}
\end{figure}

Figure \ref{buildfig} illustrates the interaction between \prog{tcng}
and the external program when building modules.

On standard input, \prog{tcng} provides configuration data in the
format described in the following section.


%------------------------------------------------------------------------------


\section{Configuration data format}
\label{extcfg}

The configuration data is passed as plain text with one statement per
line. Each line is terminated with a newline character. There are no
comments, blank lines, or leading or trailing spaces.

There are several groups of statements, each containing zero or more
statements. The groups appear in the following order:

\begin{itemize}
  \item pragma
  \item queues
  \item offsets
  \item buckets
  \item actions
  \item rules
\end{itemize}

Unless otherwise indicated, all numbers can be decimal or hexadecimal,
with the usual convention that non-zero decimal numbers have no leading zero,
and hexadecimal numbers begin with \raw{0x}.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Pragma}

If present at all, the pragma group contains only a single line:

\raw{pragma} \meta{pragmas}

\begin{description}
  \item[\meta{pragmas}] is an optional list of space-separated pragmas
\end{description}

Pragmas contain implementation-specific information for further processing
stages. In the presence of pragmas, the behaviour of any component is
undefined. Nevertheless, the external program should detect unrecognized
pragmas, and issue a warning message.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Buckets}

The buckets group defines token buckets to be used for policing. It
contains statements with the following syntax:

\raw{bucket} \meta{new\_bucket} = \meta{rate} \meta{mpu} \meta{depth}
  \meta{initial\_tokens} \meta{overflow\_bucket} \meta{pragmas}

\begin{description}
  \item[\meta{new\_bucket}] is the index of the new token bucket.
    This number must be greater than zero.
  \item[\meta{rate}] is the bucket rate in bytes per second. Tokens are
    added to the bucket at this rate.
  \item[\meta{mpu}] is the minimum policed unit, in bytes. Packets smaller
    than \meta{mpu} are treated as if they were of size \meta{mpu}.
  \item[\meta{depth}] is the depth of the token bucket in bytes, i.e. the
    maximum number of tokens, or the ``credit'', the bucket can accumulate.
  \item[\meta{initial\_tokens}] is the initial number of tokens available
    in the bucket.
  \item[\meta{overflow\_bucket}] is the number of the bucket that receives
    excess tokens from the current bucket. \meta{overflow\_bucket} is 0 if
    excess tokens will be discarded.
  \item[\meta{pragmas}] is an optional list of space-separated pragmas
\end{description}

\meta{rate} and \meta{depth} are unsigned 32-bit integers. Therefore, the
maximum values are a rate of 34 Gbps, and a depth of 4 GB, or one second
at the maximum rate.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Offsets}

The offsets group defines formulas for calculating offsets. It contains
statements with the following syntax:

\raw{offset} \meta{new\_group} = \meta{base\_group}+(\meta{field} $<<$
  \meta{shift})

\begin{description}
  \item[\meta{new\_offset}] is the index of the new offset defined here.
    This number must be greater than zero.
  \item[\meta{base\_offset}] is the index of the offset the new offset is
    based on. \meta{base\_offset} has been defined earlier in the offsets
    group. A value
    of 0 means that the new offset is relative to the beginning of the
    packet.
  \item[\meta{field}] describes the access to the field in the packet header
    containing the values used in the offset calculation. The field
    syntax is described in section \ref{rules}.
  \item[\meta{shift}] is the number of bits the value of \meta{field} is
    shifted to the left before adding it to the base offset. \meta{size}
    is greater than or equal to zero.
\end{description}

The expression yields the number of bits the new offset is displaced with
respect to the base offset.

Example:

\begin{verbatim}
offset 1 = 0+(0:0:8 << 5)
offset 2 = 1+(1:16:8 << 6)
\end{verbatim}


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Blocks}

The rest of the configuration data is divided into traffic control blocks.
Each traffic control block
begins with a block statement, followed by additional statements
describing the queues, actions, and classification rules in this block.

The block statement has the following syntax:

\raw{block} \meta{name} \meta{role} \meta{pragmas}

\begin{description}
  \item[\meta{name}] is the name of the interface.
  \item[\meta{role}] is either \raw{ingress} or \raw{egress}.
  \item[\meta{pragmas}] is an optional list of space-separated pragmas
\end{description}

Each block contains zero or more queuing disciplines, which in turn
contain zero or more classes.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Queues}
\label{extq}

Queuing disciplines are defined with the following syntax:

\raw{qdisc} \meta{index} = \meta{type} \meta{parameters} \meta{pragmas}

\begin{description}
  \item[\meta{index}] is the number of the queuing discipline
  \item[\meta{type}] is the name of the queuing discipline,
    e.g. \raw{prio} or \raw{red}
  \item[\meta{parameters}] is a space-separated list of parameters. The
    list may be empty. Each parameter consists of the parameter name,
    followed by the decimal parameter value. Parameter value conversion
    is described below. \prog{tcng} outputs parameters in alphabetic order.
  \item[\meta{pragmas}] is an optional list of space-separated pragmas,
    beginning with the keyword \raw{pragma}
\end{description}

Parameter values are converted as follows:
\begin{center}
\begin{tabular}{l|cl}
  Parameter type & \multicolumn{2}{l}{Unit in output} \\
  \hline
  Rate           & 1 Bps & (byte per second) \\
  Size in bytes  & 1 B   & (byte) \\
  Size in packets& 1 p   & (packet) \\
  Time           & 1 s   & (second) \\
  Probability    & $10^{-6}$ & (multiply with 1'000'000) \\
\end{tabular}
\end{center}

Example:

\begin{verbatim}
qdisc 2 = fifo limit 10000
\end{verbatim}


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Classes}

Class definitions immediately follow the corresponding queuing discipline,
and use the following syntax:

\raw{class} \meta{index} = \meta{parameters} \meta{pragmas}

\begin{description}
  \item[\meta{index}] is the number of the class
  \item[\meta{parameters}] is a space-separated list of parameters. Parameter
   value conversion is described in section \ref{extq}.
  \item[\meta{pragmas}] is an optional list of space-separated pragmas,
    beginning with the keyword \raw{pragma}
\end{description}

Example:

\begin{verbatim}
qdisc 2 = prio bands 3
class 2 = qdisc 4
class 1 = qdisc 3
\end{verbatim}

In order to express the hierarchy of queuing disciplines and classes,
two pseudo-parameters are added to class definitions:

\begin{description}
  \item[\raw{qdisc}] specifies the number of the
    queuing discipline attached to this class
% t2x has a problem here
  \item[\name{parent}] specifies the number of the parent class of this class.
    If the \raw{parent} parameter is absent, the class is at the top level of
    the class hierarchy.
\end{description}


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Actions}

The actions group defines actions to be taken when packets match a
certain bit pattern (the matching rules are defined in the rules
group, described in the next section). It contains statements with the
following syntax:

\raw{action} \meta{new\_action} = \meta{action}

\begin{description}
  \item[\meta{new\_action}] is the index of the new action.
  \item[\meta{action}] is one of

    \raw{class} \meta{qdisc}\raw{:}\meta{class},$\ldots$

    \raw{drop}

    \raw{unspec}

    \raw{conform} \meta{bucket} \meta{yes\_action} \meta{no\_action}

    \raw{count} \meta{bucket} \meta{next\_action}

    \raw{action} \meta{action}

    \begin{description}
      \item[\raw{class}] selects the corresponding class or classes.
	\meta{qdisc} is the index of the queuing discipline.
	\meta{class} is the number of the class within the queuing discipline.
	If \prog{tcng} is able to identify the sequence of queues and classes
	visited
	by a packet when taking this action, the whole sequence is described
	in a comma-separated list of queue and class numbers, starting with
	the outermost queue.
      \item[\raw{drop}] indicates that the packet shall be dropped.
      \item[\raw{unspec}] indicates that the packet was not recognized by the
	classifier, and that the default action of the queuing discipline
	should be taken. Note that the meaning of \raw{unspec} changes when
        using \name{nocombine}, see section \ref{barriers}.
      \item[\raw{conform}] tests whether the current packet can be accepted
	by the token bucket number \meta{bucket}, and executes
	\meta{yes\_action} if it does, \meta{no\_action} otherwise.
	Note that \raw{conform} does \emphasize{not} remove tokens from the
	bucket.
      \item[\raw{count}] removes tokens corresponding to the current packet
	from token bucket \meta{bucket}, and continues with
	\meta{next\_action}. If the bucket contains fewer tokens than required
	for the current packet, \raw{count} sets the bucket to zero.
    \end{description}
\end{description}

\meta{yes\_action}, \meta{no\_action}, and \meta{next\_action} are of the
form

\raw{action} \meta{action}

which continues with the action statement with index \meta{meta}.

Example:

\begin{verbatim}
action 0 = drop
action 1 = class 1:1
action 2 = class 1:2
action 3 = count 1 action 2
action 4 = conform 1 action 3 action 0
\end{verbatim}

Historical note: action statements used to contain more than one action,
and the syntax still reflects this original design. This was changed in
order to simplify the data structures in programs using the external
interface.

Implementation note: while, in theory, tokens are continuously added
to a bucket, an implementation will typically choose to add tokens only
at packet arrival, and possibly only to buckets considered for the
current packet. In this context, it is important to consider that
\raw{count} may be used without prior \raw{conform}, so adding tokens
only on \raw{conform} would not yield correct behaviour.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Rules}
\label{rules}

The rules group defines the matching rules for static packet classification.
It contains statements with the following syntax:

\raw{match} \meta{field}=\meta{value} $\ldots$ \raw{action} \meta{action}

Each rule has zero or more \meta{field}=\meta{value} entries. The rule
matches if and only if all fields have the requested value. If the
rule matches, the corresponding action is executed. Otherwise,
matching continues with the next rule.

\begin{description}
  \item[\meta{field}] specifies the field to read. A field
    specification has the following syntax:

    \meta{offset\_group}:\meta{offset}:\meta{length}

    This extracts \meta{length} bits starting at the position defined
    by the offset of \meta{offset\_group}, plus \meta{offset} bits. The
    first bit in the packet has offset zero. If \meta{length} is zero,
    the expression always yields 0, and no actual access should be made.
    A field can be arbitrarily long, i.e. larger than 32 bits.
  \item[\meta{value}] is the value which the field must contain. The value
    must be a hexadecimal number, and can be of arbitrary length (i.e.
    larger than the native word size of the machine).
  \item[\meta{action}] is the index of the action to take if the rule
    matches.
\end{description}

Matches are processed in the order specified. If a rule contains
further matches after a mismatch is found, these entries are ignored,
and no access to packet data is performed for them.

Examples:

\begin{verbatim}
match 0:72:8=0x11 1:16:16=0x829A action 1
\end{verbatim}


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Barriers}
\label{barriers}

When using the \name{nocombine} configuration option, \prog{tcng} divides
the rules into self-contained subsets, which are separated by lines
with the word

\raw{barrier}

When iterating through rules, i.e. when looking for a match, barriers
can be ignored. However, they change the meaning of the action
\name{unspec}: when encountering \name{unspec}, the classifier has to
look for a barrier following the current rule, and proceed with the
next rule following this barrier. Only if there is no barrier following
the current rule can the classifier return \name{unspec} as the final result.

Example:

\begin{tabular}{rlcc}
  Line & Action/rule & \multicolumn{2}{c}{Next rule} \\
  & & Match & No match \\
  \hline
  1 & \verb"action 0 = unspec" & & \\
  2 & \verb"action 1 = class 1:1" & & \\
  3 & $\ldots$ & & \\
  4 & \verb"match ... action 1" & return & 5 \\
  5 & \verb"match ... action 0" & 8 & 6 \\
  6 & \verb"match action 1" & return & --- \\
  7 & \verb"barrier" & 8 & \\
  8 & \verb"match ... action 1" & return & 9 \\
  9 & \verb"match action 0" & return & --- \\
\end{tabular}


%------------------------------------------------------------------------------


\section{Data returned}

If the external program experiences a problem, it should return with a
non-zero exit code and may print error messages on standard error.
Any output written to standard output before termination must conform
to the rules below.

Otherwise, it must return with exit code zero, may print diagnostics on
standard error, and must provide additional data
on standard output. This output is different for different operations,
as described in the following sections.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Configuration query}
\label{cfgqout}

The program must print a list of zero or more key phrases, one per line,
on standard output.\footnote{While we use
the expression key ``phrase'', all currently recognized phrases are
actually single words. This may change in the future.}

The following key phrases are recognized by \prog{tcng}:

\begin{description}
  \item[\name{all}] external program handles the complete traffic control
    subsystem configuration, including device setup, queuing, etc. This
    mode can also be enabled by specifying the external program with
    \raw{-x all:\meta{external\_target}}
  \item[\name{debug\_target}] target has only debugging functions and does
    not generate any elements
  \item[\name{fifos}] \prog{tcng} automatically adds a FIFO queuing discipline
    to each
    class that could have a child queuing discipline, if no queuing
    discipline is explicitly configured there
  \item[\name{nocombine}] do not combine individual \name{if} clauses into
    a single classifier. This can tremendously accelerate the processing
    done by \prog{tcng}, by offloading work to the external program.
    The implications of this are discussed in more detail in section
    \ref{barriers}.
  \item[\name{nocontinue}] this key phrase is only recognized for
    backward compatibility and has no effect
  \item[\name{nounspec}] the external program does not handle the ``unspec''
    classification result in queuing disciplines, so \prog{tcng} needs to
    generate rules to implement default actions of queuing disciplines
\end{description}


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Capability probe}

At the moment, \prog{tcng} does not invoke capability probes.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Build run}
\label{buildrun}

In the build run, the external program must return the names of all traffic
control elements used to implement the configuration. Each line must have
the following structure:

\meta{element\_type} \meta{element\_name} \meta{tc\_arguments} $\ldots$

\begin{description}
  \item[\meta{element\_type}] is either \raw{qdisc} or \raw{filter}.
  \item[\meta{element\_name}] is a name that uniquely identifies the
    element in question. When creating a queuing discipline, the
    kernel module must have the name

    \raw{sch\_}\meta{element\_name}\raw{.o}

    and the tc module must have the name

    \raw{q\_}\meta{element\_name}\raw{.so}

    Likewise, for a filter, the file names are
    \raw{cls\_}\meta{element\_name}\raw{.o} and
    \raw{f\_}\meta{element\_name}\raw{.so}
  \item[\meta{tc\_arguments}] are zero or more arguments that are
    passed to the tc command.
\end{description}

Note that the external program must return at least one element,
except if operating in \name{all} mode, or if it has returned the
key phrase \name{debug\_target} in the configuration query.


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


\subsection{Diagnostic message translation}

When an external program reports a problem with a traffic control element,
it can include an identifier, which is then translated to the source code
location by \prog{tcng}.

For this purpose, \prog{tcng} reads standard error of the external
program, and tries to translate all strings of the form
\verb"[<"\meta{type}\verb"@"\meta{identifier}\verb">]", where \meta{verb} is
one of the element type names described in section \ref{locfile},
and the identifier is the corresponding element identifier described
in that same section.


%------------------------------------------------------------------------------


\section{C API}

\prog{tcng} provides a library called \name{tccext} that provides functions
to parse the output generated by \prog{tcng}, and to store the result in C
structures which can easily be accessed in programs.

The library is normally installed as \url{/usr/lib/tcng/lib/libtccext.a}
(\url{tcng/lib/tcng/lib/libtccext.a} if using a local copy).

The data structures and functions provided by this library are described
in the accompanying header file \url{tccext.h}, which is installed in
\url{/usr/lib/tcng/include/tccext.h} (\url{tcng/tcc/ext/tccext.h} if
using a local copy).


%==============================================================================
