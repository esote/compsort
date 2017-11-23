#include <algorithm> // std::find_if
#include <boost/program_options.hpp>
#include <exception> // std::exception
#include <iomanip> // std::setew
#include <iostream>
#include <limits> // std::numeric_limits
#include <random>
#include <tuple>
#include <type_traits> // std::is_floating_point
#include <utility> //std::reference_wrapper, std::binary_function
#include <vector>

template<typename VT>
struct program_args {
	// general
	bool help, quiet, time;
	int prec = std::numeric_limits<VT>::max_digits10;
	long long avg, avg_count = 0;
	std::string delim;

	// fill
	std::vector<VT> list;
	long long fill_rand, fill_forward = 0, fill_backward = 0;
	VT fill_rand_lower, fill_rand_upper, fill_increment;

	// algorithm
	bool alg_all;
	std::vector<std::string> alg_except;

	typedef typename std::tuple<const std::string, const std::string, double, bool> TUP;

	TUP bogosort = mt("bogosort", "Bogosort: "),
		bubble_sort = mt("bubble-sort", "Bubble Sort: "),
		cocktail_sort = mt("cocktail-sort", "Cocktail Sort: "),
		gnome_sort = mt("gnome-sort", "Gnome Sort: "),
		heap_sort = mt("heap-sort", "Heap Sort: "),
		insertion_sort = mt("insertion-sort", "Insertion Sort: "),
		merge_sort = mt("merge-sort", "Merge Sort: "),
		permutation_sort = mt("permutation-sort", "Permutation Sort: "),
		quick_sort = mt("quick-sort", "Quick Sort: "),
		selection_sort = mt("selection-sort", "Selection Sort: ");

	const std::vector<std::reference_wrapper<TUP> > algs = {
		bogosort,
		bubble_sort,
		cocktail_sort,
		gnome_sort,
		heap_sort,
		insertion_sort,
		merge_sort,
		permutation_sort,
		quick_sort,
		selection_sort
	};

	const int mstr = static_cast<int>(max_strlen(algs));

	private:
		TUP mt(const std::string & str1, const std::string & str2,
			const double & sum = 0.0, const bool & use = false) {
			return std::make_tuple(str1, str2, sum, use);
		}

		template<typename T>
		std::size_t max_strlen(const T & vect) {
			std::size_t len = 0;
			for(const auto & i : vect) {
				std::size_t j = std::get<1>(i.get()).length();
				len = (j > len) ? j : len;
			}
			return len;
		}
};

enum returnID {
	success_help = -1,
	success = 0,

	prec_under = 1,
	prec_over = 2,

	avg_under = 3,

	fill_rand_under = 4,
	fill_forward_under = 5,
	fill_backward_under = 6,

	alg_empty = 7,
	alg_invalid = 8,

	known_err = 9,
	other_err = 10
};

void conflicting_opts(const boost::program_options::variables_map & vm, 
	const std::string & opt1, const std::string & opt2) {
	if(vm.count(opt1) && !vm[opt1].defaulted()
		&& vm.count(opt2) && !vm[opt2].defaulted())
		throw std::logic_error(std::string("Conflicting options: '--")
			+ opt1 + "' and '--" + opt2 + "'.");
}

template<typename VT>
returnID parse_args(program_args<VT> & args, const int & argc, char const * const * argv) {
	namespace po = boost::program_options;

	po::options_description general("General options");
	general.add_options()
		("help", po::bool_switch(&args.help),
			"produce this help message")
		("quiet", po::bool_switch(&args.quiet),
			"disable printing sorted list")
		("time", po::bool_switch(&args.time),
			"print CPU time for each algorithm")
		("prec", po::value<int>(&args.prec)->default_value(args.prec),
			"set OUTPUT precision")
		("avg", po::value<long long>(&args.avg)->default_value(1),
			"rerun sorting a specified number times on the same list")
		("delim", po::value<std::string>(&args.delim)->default_value(" "),
			"delimiters used when printing lists, "
			"wrap in quotation marks, escape characters as needed");

	po::options_description fill("Fill options");
	fill.add_options()
		("list", po::value<std::vector<VT> >(&args.list)->multitoken(),
			"input a list of values")
		("fill-rand", po::value<long long>(&args.fill_rand)->default_value(10),
			"fill the list random numbers")
		("rand-lower", po::value<VT>(&args.fill_rand_lower)->default_value(-10),
			"specify the lower bound for '--fill-rand'")
		("rand-upper", po::value<VT>(&args.fill_rand_upper)->default_value(10),
			"specify the upper bound for '--fill-rand'")
		("fill-forward", po::value<long long>(&args.fill_forward),
			"fill the list with incrementing numbers")
		("fill-backward", po::value<long long>(&args.fill_backward),
			"fill the list with decrementing numbers")
		("fill-increment", po::value<VT>(&args.fill_increment)->default_value(1),
			"specify the fill increment used with '--fill-forward' and '--fill-backward'");

	po::options_description algorithm("Algorithm options");
	algorithm.add_options()
		("alg-all", po::bool_switch(&args.alg_all),
			"use all available algorithms")
		("alg-except", po::value<std::vector<std::string> >(&args.alg_except)->multitoken(),
			"except algorithms when using '--alg-all', wrap each argument in quotation marks")
		("alg-bogosort", po::bool_switch(&std::get<3>(args.bogosort)),
			"use the bogosort algorithm")
		("alg-bubble-sort", po::bool_switch(&std::get<3>(args.bubble_sort)),
			"use the bubble sort algorithm")
		("alg-cocktail-sort", po::bool_switch(&std::get<3>(args.cocktail_sort)),
			"use the cocktail shaker sort")
		("alg-gnome-sort", po::bool_switch(&std::get<3>(args.gnome_sort)),
			"use the gnome sort algorithm")
		("alg-heap-sort", po::bool_switch(&std::get<3>(args.heap_sort)),
			"use the heap sort algorithm")
		("alg-insertion-sort", po::bool_switch(&std::get<3>(args.insertion_sort)),
			"use the insertion sort algorithm")
		("alg-merge-sort", po::bool_switch(&std::get<3>(args.merge_sort)),
			"use the merge sort algorithm")
		("alg-permutation-sort", po::bool_switch(&std::get<3>(args.permutation_sort)),
			"use the permutation sort algorithm")
		("alg-quick-sort", po::bool_switch(&std::get<3>(args.quick_sort)),
			"use the quick sort algorithm")
		("alg-selection-sort", po::bool_switch(&std::get<3>(args.selection_sort)),
			"use the selection sort algorithm");

	po::options_description all("Allowed options");
	all.add(general).add(fill).add(algorithm);

	po::variables_map vm;
	po::store(parse_command_line(argc, argv, all, po::command_line_style::unix_style 
		^ po::command_line_style::allow_short), vm);
	po::notify(vm);

	if(args.help) {
		std::cout << all << '\n';
		return returnID::success_help;
	}

	// general
	if(args.prec < 0) {
		std::cerr << "error: '--prec' cannot be < 0\n";
		return returnID::prec_under;

	} else if(args.prec > std::numeric_limits<VT>::max_digits10) {
		std::cerr << "error: '--prec' cannot be greater than the data type's precision ("
			<< std::numeric_limits<VT>::max_digits10 << ")\n";
		return returnID::prec_over;
	}

	if(args.avg <= 0) {
		std::cerr << "error: '--avg' cannot be <= 0\n";
		return returnID::avg_under;
	}

	// fill
	conflicting_opts(vm, "list", "fill-rand");
	conflicting_opts(vm, "list", "fill-forward");
	conflicting_opts(vm, "list", "fill-backward");
	conflicting_opts(vm, "list", "fill-increment");

	conflicting_opts(vm, "fill-rand", "fill-forward");
	conflicting_opts(vm, "fill-rand", "fill-backward");
	conflicting_opts(vm, "fill-rand", "fill-increment");

	conflicting_opts(vm, "fill-forward", "fill-backward");

	if(args.fill_rand < 0) {
		std::cerr << "error: '--fill-rand' must be >= 0\n";
		return returnID::fill_rand_under;
	} else if(vm.count("fill-rand") && !vm.count("fill-forward") && !vm.count("fill-backward") && !vm.count("list")) {
		fill_rand(args);
	}

	if(args.fill_forward < 0) {
		std::cerr << "error: '--fill-forward' must be >= 0\n";
		return returnID::fill_forward_under;
	} else if(vm.count("fill-forward")) {
		fill_forward(args);
	}

	if(args.fill_backward < 0) {
		std::cerr << "error: 'fill-backward' must be >= 0\n";
		return returnID::fill_backward_under;
	} else if(vm.count("fill-backward")) {
		fill_backward(args);
	}

	// algorithm
	for(auto & i : args.algs) {
		conflicting_opts(vm, "alg-except", "alg-" + std::get<0>(i.get()));
	}

	for(const auto & i : args.alg_except) {
		if(i.compare(0, 2, "--") == 0) {
			std::cerr << "error: the required argument for option '--alg-except' is missing\n";
			return returnID::alg_empty;
		}
		if(std::find_if(args.algs.begin(), args.algs.end(),
			[i](const auto & e) { return std::get<0>(e.get()) == i; }) == args.algs.end()) {
			std::cerr << "error: the argument ('"
				<< i << "') for option '--alg-except' is invalid\n";
			return returnID::alg_invalid;	
		}
	}

	return returnID::success;
}

template<typename VT>
void printVect(const std::vector<VT> & list, const int & prec, const std::string & delim) {
	std::cout.precision(prec);
	for(const VT & i : list) {
		std::cout << std::fixed << i << delim;
	}
}

// begin: helpers
template<typename Order>
struct non_strict_op:
	std::binary_function<typename Order::second_argument_type,
		typename Order::first_argument_type, bool> {
			non_strict_op(Order o): order(o) {}
			bool operator()(typename Order::second_argument_type arg1,
				typename Order::first_argument_type arg2) const {
					return !order(arg2, arg1);
				}
	private:
		Order order;
};
 
template<typename Order> non_strict_op<Order> non_strict(Order o) {
	return non_strict_op<Order>(o);
}
// end: helpers

// begin: algorithms
template<typename It>
void bogosort(It first, It last) {
	std::mt19937 generator{std::random_device{}()};
	while(!std::is_sorted(first, last, 
		std::less<typename std::iterator_traits<It>::value_type>())) {
		std::shuffle(first, last, generator);
	}
}

template<typename It>
void bubble_sort(It first, It last) {
	bool swapped = true;
	while(first != last-- && swapped) {
		swapped = false;
		for(auto i = first; i != last; ++i) {
			if(*(i + 1) < *i) {
				std::iter_swap(i, i + 1);
				swapped = true;
			}
		}
	}
}

template<typename It>
void cocktail_sort(It first, It last) {
	bool swapped = true;
	while(first != last-- && swapped) {
		swapped = false;
		for(auto i = first; i != last; ++i) {
			if(*(i + 1) < *i) {
				std::iter_swap(i, i + 1);
				swapped = true;
			}
		}
		if(!swapped) break;
		swapped = false;
		for(auto i = last - 1; i != first; --i) {
			if(*i < *(i - 1)) {
				std::iter_swap(i, i - 1);
				swapped = true;
			}
		}
		++first;
	}
}

template<typename It>
void gnome_sort(It first, It last) {
	auto i = first + 1;
	auto j = first + 2;
	while(i < last) {
		if(!(*i < *(i - 1))) {
			i = j;
			++j;
		} else {
			std::iter_swap(i - 1, i);
			--i;
			if(i == first) {
				i = j;
				++j;
			}
		}
	}
}

template<typename It>
void heap_sort(It first, It last) {
	std::make_heap(first, last);
	std::sort_heap(first, last);
}

template<typename It>
void insertion_sort(It first, It last) {
	for(auto i = first; i != last; ++i) {
		std::rotate(std::upper_bound(first, i, *i,
			std::less<typename std::iterator_traits<It>::value_type>()), i, i + 1);
	}
}

template<typename It>
void merge_sort(It first, It last) {
	if(last - first > 1) {
		It middle = first + (last - first) /  2;
		merge_sort(first, middle);
		merge_sort(middle, last);
		std::inplace_merge(first, middle, last);
	}
}

template<typename It>
void permutation_sort(It first, It last) {
	while(std::next_permutation(first, last)) {}
}

template<typename It>
 void quick_sort(It first, It last)
{
  if (first != last && first+1 != last)
  {
    typedef typename std::iterator_traits<It>::value_type value_type;
    It mid = first + (last - first)/2;

    // median of 3
    value_type pivot = std::max(std::min(*first, *mid), 
    	std::min(std::max(*first, *mid), *(last-1)));

    It split1 = std::partition(first, last, 
    	std::bind2nd(std::less<value_type>(), pivot));
    It split2 = std::partition(split1, last, 
    	std::bind2nd(non_strict(std::less<value_type>()), pivot));
    quick_sort(first, split1);
    quick_sort(split2, last);
  }
}

template<typename It>
void selection_sort(It first, It last) {
	for(auto i = first; i != last; ++i) {
		std::iter_swap(i, std::min_element(i, last));
	}
}
// end: algorithms

template<typename T>
bool contains(const std::vector<T> & vect, const std::string & alg) {
	return std::find(vect.begin(), vect.end(), alg) != vect.end();
}

template<typename VT, typename FT>
void sort(const program_args<VT> & args, FT && sorter, const std::string & msg, double & sum) {
	auto list_cp = args.list;

	std::clock_t tstart = clock();

	static_cast<FT &&>(sorter)(list_cp.begin(), list_cp.end());

	const double time = (double)(clock() - tstart) / CLOCKS_PER_SEC;
	sum += time;

	if(!args.quiet && (args.avg == 1 || args.avg_count == (args.avg - 1)))
		std::cout << "\n\n";

	if(args.avg == 1 || args.avg_count == (args.avg - 1))
		std::cout << std::left << std::setw(args.mstr) << msg;

	if(args.time && args.avg == 1) {
		std::cout.precision(6);
		std::cout << std::fixed << "CPU time: " << time << " s";
	} else if(args.time && args.avg_count == (args.avg - 1)) {
		std::cout.precision(6);
		std::cout << std::fixed << "Average CPU time: " << sum / (double)(args.avg) << " s";
	} 

	if (args.avg == 1 || args.avg_count == (args.avg - 1)) {
		std::cout << '\n';
	}
	
	if(!args.quiet && (args.avg == 1 || args.avg_count == (args.avg - 1))) {
		printVect(list_cp, args.prec, args.delim);
	}
}

// begin: fillers
template<typename VT>
void fill_rand(program_args<VT> & args) {
	typedef typename std::conditional<std::is_floating_point<VT>::value,
		std::uniform_real_distribution<VT>,
		std::uniform_int_distribution<VT> >::type DIST;

	std::mt19937 generator{std::random_device{}()};
	DIST dis{args.fill_rand_lower, args.fill_rand_upper};

	for(long long i = 0; i < args.fill_rand; ++i)
		args.list.push_back(dis(generator));
}

template<typename VT>
void fill_forward(program_args<VT> & args) {
	VT count = 0;
	for(long long i = 0; i < args.fill_forward; ++i, count += args.fill_increment)
		args.list.push_back(count);
}

template<typename VT>
void fill_backward(program_args<VT> & args) {
	VT count = 0;
	for(long long i = 0; i < args.fill_backward; ++i, count += args.fill_increment)
		args.list.insert(args.list.begin(), count);
}
// end: fillers

int main(int argc, char * argv[]) {
	try {
		// change to required type
		program_args<long double> args;

		switch(auto result = parse_args(args, argc, argv)) {
			case returnID::success: break;
			case returnID::success_help: return 0;
			default: return result;
		}

		if(!args.quiet) {
			std::cout << "Before:\n";
			printVect(args.list, args.prec, args.delim);
		}

		typedef typename std::vector<decltype(args.list)::value_type>::iterator IT;

		std::vector<std::function<void(IT, IT)> > lambs = {
			[](auto f, auto l) { bogosort(f, l);},
			[](auto f, auto l) { bubble_sort(f, l);},
			[](auto f, auto l) { cocktail_sort(f, l);},
			[](auto f, auto l) { gnome_sort(f, l);},
			[](auto f, auto l) { heap_sort(f, l);},
			[](auto f, auto l) { insertion_sort(f, l);},
			[](auto f, auto l) { merge_sort(f, l);},
			[](auto f, auto l) { permutation_sort(f, l);},
			[](auto f, auto l) { quick_sort(f, l);},
			[](auto f, auto l) { selection_sort(f, l);}
		};

		for(; args.avg_count < args.avg; ++args.avg_count) {
			long long i = 0;

			for(const auto & j : args.algs) {
				if((args.alg_all || std::get<3>(j.get()))
					&& !contains(args.alg_except, std::get<0>(j.get()))) {
					sort(args, lambs[i], std::get<1>(j.get()), std::get<2>(j.get()));
				}
				++i;
			}
		}

		if(!args.quiet) std::cout << '\n';

		return returnID::success;
	} catch(std::exception & e) {
		std::cerr << "error: " << e.what() << '\n';
		return returnID::known_err;
	} catch(...) {
		std::cerr << "error: exception of unknown type!\n";
		return returnID::other_err;
	}
}
