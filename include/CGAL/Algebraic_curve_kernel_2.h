// TODO: Add licence
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL:$
// $Id: $
// 
//
// Author(s)     : Eric Berberich <eric@mpi-inf.mpg.de>
//                 Pavel Emeliyanenko <asm@mpi-sb.mpg.de>
//                 Michael Kerber <mkerber@mpi-inf.mpg.de>
//
// ============================================================================

/*! \file Algebraic_curve_kernel_2.h
 *  \brief defines class \c Algebraic_curve_kernel_2
 *
 * A model for CGAL's AlgebraicKernelWithAnalysis_d_2 concept
 */

#ifndef CGAL_ALGEBRAIC_CURVE_KERNEL_2_H
#define CGAL_ALGEBRAIC_CURVE_KERNEL_2_H

#include <CGAL/config.h>
#include <CGAL/Algebraic_curve_kernel_2/flags.h>
#include <CGAL/Algebraic_kernel_1.h>

#include <CGAL/Algebraic_curve_kernel_2/LRU_hashed_map.h>
#include <CGAL/Algebraic_curve_kernel_2/Xy_coordinate_2.h>
//#include <CGAL/Algebraic_curve_kernel_2/Algebraic_real_traits.h>

#include <CGAL/Algebraic_curve_kernel_2/trigonometric_approximation.h>

#include <CGAL/Polynomial_type_generator.h>
#include <CGAL/polynomial_utils.h>

#if CGAL_ACK_USE_EXACUS
#include <CGAL/Algebraic_curve_kernel_2/Curve_analysis_2_exacus.h>
#include <CGAL/Algebraic_curve_kernel_2/Curve_pair_analysis_2_exacus.h>
#else
#include <CGAL/Algebraic_curve_kernel_2/Curve_analysis_2.h>
#include <CGAL/Algebraic_curve_kernel_2/Curve_pair_analysis_2.h>
#endif

#include <boost/shared_ptr.hpp>


namespace CGAL {


/*!
 * \b Algebraic_curve_kernel_2 is a model of CGAL's concept \c
 * AlgebraicKernelWithAnalysis_d_2 which itself refines \c AlgebraicKernel_d_2.
 * As such, it contains functionality
 * for solving and manipulating (systems of) bivariate polynomials,
 * of arbitrary degree,
 * as required by the \c AlgebraicKernel_d_2 concept. 
 * Additionally, it contains functionality for the topological-geometric
 * analysis of a single algebraic curve 
 * (given as the vanishing set of the polynomial), 
 * and of a pair of curves (given as a pair of polynomials), as required by the
 * \c AlgebraicKernelWithAnalysis_d_2 concept. These two analyses are
 * available via the types \c Curve_analysis_2 and Curve_pair_analysis_2.
 *
 * The given class is also a model of the \c CurveKernel_2 concept that is
 * in turn required by the \c CurvedKernelViaAnalysis_2 concept
 * (see the documentation of the corresponding package). Therefore,
 * some types and methods of the class have both an "algebraic" name
 * (demanded by \c CurveKernelWithAnalysis_d_2) and an "non-algebraic name
 * (demanded by \c CurveKernel_2).
 *
 * \b Algebraic_curve_kernel_2 is a template class, and needs a model
 * of the \c AlgebraicKernel_d_1 concept as parameter.
 *
 * Internally, the curve- and curve-pair analysis 
 * are the computational fundament of the kernel. That means, whenever
 * a polynomial is considered within the kernel, the curve analysis
 * of the corresponding algebraic curve is performed.
 * The same holds for the curve pair analysis,
 * when a kernel function deals with two polynomials,
 * implicitly or explicitly (e.g. \c Solve_2, \c Sign_at_2).
 */
#if CGAL_ACK_USE_EXACUS
template < class AlgebraicCurvePair_2, class AlgebraicKernel_d_1 >
#else
template < class AlgebraicKernel_d_1 >
#endif
class Algebraic_curve_kernel_2 : public AlgebraicKernel_d_1{

// for each predicate functor defines a member function returning an instance
// of this predicate
#define CGAL_Algebraic_Kernel_pred(Y,Z) \
    Y Z() const { return Y((const Algebraic_kernel_2*)this); }

// the same for construction functors
#define CGAL_Algebraic_Kernel_cons(Y,Z) CGAL_Algebraic_Kernel_pred(Y,Z)

protected:    
    // temporary types
    
public:
    //!\name public typedefs
    //!@{

    //! type of 1D algebraic kernel
    typedef AlgebraicKernel_d_1 Algebraic_kernel_1;
    
#if CGAL_ACK_USE_EXACUS    
    // type of an internal curve pair
    typedef AlgebraicCurvePair_2 Internal_curve_pair_2;
    
    // type of an internal curve
    typedef typename AlgebraicCurvePair_2::Algebraic_curve_2 Internal_curve_2;
#endif

    //! type of x-coordinate
#if CGAL_ACK_USE_EXACUS
    typedef typename Internal_curve_2::X_coordinate X_coordinate_1;
#else
    typedef typename Algebraic_kernel_1::Algebraic_real_1 X_coordinate_1;
#endif

    //! type of y-coordinate
    typedef X_coordinate_1 Y_coordinate_1;
    
    //! type of polynomial coefficient
    typedef typename Algebraic_kernel_1::Coefficient Coefficient;

    // myself
#if CGAL_ACK_USE_EXACUS
    typedef Algebraic_curve_kernel_2<AlgebraicCurvePair_2, AlgebraicKernel_d_1>
       Self;
#else
    typedef Algebraic_curve_kernel_2<AlgebraicKernel_d_1> Self;
#endif

    typedef Self Algebraic_kernel_2;
    
    // Bound type
    typedef typename Algebraic_kernel_1::Bound Bound;
        
    typedef typename CGAL::Get_arithmetic_kernel<Bound>::Arithmetic_kernel
      Arithmetic_kernel;
    
    typedef typename Arithmetic_kernel::Bigfloat Bigfloat;
    typedef typename Arithmetic_kernel::Bigfloat_interval Bigfloat_interval;
    
    //! Univariate polynomial type 
    typedef typename Algebraic_kernel_1::Polynomial_1 Polynomial_1;
    
    //! Bivariate polynomial type
    typedef typename CGAL::Polynomial_traits_d<Polynomial_1>
    :: template Rebind<Coefficient,2>::Other::Type Polynomial_2;
    
    //! bivariate polynomial traits
    typedef ::CGAL::Polynomial_traits_d< Polynomial_2 >
        Polynomial_traits_2;

    /*!
     * \brief  type of a curve point, a model for the 
     * \c AlgebraicKernel_d_2::AlgebraicReal_2 concept
     */
    typedef internal::Xy_coordinate_2<Self> Xy_coordinate_2;

    /*! 
     * type of the curve analysis, a model for the
     * \c AlgebraicKernelWithAnalysis_d_2::CurveAnalysis_2 concept
     */
#if CGAL_ACK_USE_EXACUS
    typedef internal::Curve_analysis_2<Self> Curve_analysis_2; 
#else
    typedef CGAL::Curve_analysis_2<Self> Curve_analysis_2; 
#endif

    /*! 
     * type of the curve pair analysis, a model for the
     * \c AlgebraicKernelWithAnalysis_d_2::CurvePairAnalysis_2 concept
     */
#if CGAL_ACK_USE_EXACUS
    typedef internal::Curve_pair_analysis_2<Self> Curve_pair_analysis_2;
#else
    typedef CGAL::Curve_pair_analysis_2<Self> Curve_pair_analysis_2;
#endif

    //! traits class used for approximations of y-coordinates


    //  berfriending representations to make protected typedefs available
    friend class internal::Curve_analysis_2_rep<Self>;
    friend class internal::Curve_pair_analysis_2_rep<Self>;
    
    //!@}
    //! \name rebind operator
    //!@{
#if CGAL_ACK_USE_EXACUS
    template <class NewCurvePair, class NewAlgebraicKernel>
    struct rebind {
        typedef Algebraic_curve_kernel_2<NewCurvePair,NewAlgebraicKernel> 
            Other;
    };
#else
    template <class NewAlgebraicKernel> 
    struct rebind { 
        typedef Algebraic_curve_kernel_2<NewAlgebraicKernel> Other;        
    };
#endif

    //!@}
protected:
    //! \name private functors
    //!@{
 
#if 0
   
    //! polynomial canonicalizer, needed for the cache
    template <class Poly> 
    struct Poly_canonicalizer : public std::unary_function< Poly, Poly >
    {
    // use Polynomial_traits_d<>::Canonicalize ?
        Poly operator()(Poly p) 
        {
            typedef CGAL::Scalar_factor_traits<Poly> Sf_traits;
            typedef typename Sf_traits::Scalar Scalar;
            typename Sf_traits::Scalar_factor scalar_factor;
            typename Sf_traits::Scalar_div scalar_div;
            Scalar g = scalar_factor(p);
            if (g == Scalar(0)) {
                     CGAL_assertion(p == Poly(Scalar(0)));
                     return p;
            }
            CGAL_assertion(g != Scalar(0));
            if(g != Scalar(1)) 
                scalar_div(p,g);
            if(CGAL::leading_coefficient(CGAL::leading_coefficient(p))) < 0) 
                scalar_div(p,Scalar(-1));
            return p;        
        }
           
    };
#endif

    // NOT a curve pair in our notation, simply a std::pair of Curve_analysis_2
    typedef std::pair<Curve_analysis_2, Curve_analysis_2> Pair_of_curves_2;
    
    //! orders pair items by ids
    struct Pair_id_order {

        template<class T1, class T2>
        std::pair<T1, T2> operator()(const std::pair<T1, T2>& p) const {
            
            if(p.first.id() > p.second.id())
                return std::make_pair(p.second, p.first);
            return p;
        }
    };
    
    class Curve_creator {

    public:

        Curve_creator(Algebraic_kernel_2* kernel) : _m_kernel(kernel) {}
        Curve_analysis_2 operator()(const Polynomial_2& f) const {
          return Curve_analysis_2(_m_kernel,f);
        }

    protected:
        
        Algebraic_kernel_2* _m_kernel;
        
    };

    template <class Result>
    class Pair_creator {

    public:

        Pair_creator(Algebraic_kernel_2* kernel) : _m_kernel(kernel) {}

        template<class T1, class T2>
        Result operator()(const std::pair<T1, T2>& p) const {
            return Result(_m_kernel, p.first, p.second);
        }

    protected:
        
        Algebraic_kernel_2* _m_kernel;
        
    };
    
    struct Pair_id_equal_to {

        template <class T1, class T2>
        bool operator()(const std::pair<T1, T2>& p1,
                const std::pair<T1, T2>& p2) const {
            return (p1.first.id() == p2.first.id() &&
                 p1.second.id() == p2.second.id());
        }
    };

    //! type of curve analysis cache
    typedef internal::LRU_hashed_map_with_kernel<Self,Polynomial_2,
        Curve_analysis_2, internal::Poly_hasher,
        std::equal_to<Polynomial_2>,
        typename Polynomial_traits_2::Canonicalize,
        Curve_creator > Curve_cache_2;

    //! type of curve pair analysis cache 
    typedef internal::LRU_hashed_map_with_kernel<Self,Pair_of_curves_2,
        Curve_pair_analysis_2, internal::Pair_hasher, Pair_id_equal_to,
        Pair_id_order,
        Pair_creator<Curve_pair_analysis_2> > Curve_pair_cache_2;
    
    typedef std::pair<Polynomial_2, Polynomial_2>
        Pair_of_polynomial_2;

    template<typename T> struct Gcd {
    
        T operator() (std::pair<T,T> pair) {
            return typename CGAL::Polynomial_traits_d<Polynomial_2>
                ::Gcd_up_to_constant_factor()(pair.first,pair.second);
        }
    } ;     


    template<typename T> struct Pair_cannonicalize {
    
        std::pair<T,T> operator() (std::pair<T,T> pair) {
        
            if(pair.first > pair.second) 
                return std::make_pair(pair.second,pair.first);
            return pair;
        }
    };

    typedef CGAL::Pair_lexicographical_less_than
    <Polynomial_2, Polynomial_2,
            std::less<Polynomial_2>,
            std::less<Polynomial_2> > Polynomial_2_compare;
    
    //! Cache for gcd computations
    typedef CGAL::Cache<Pair_of_polynomial_2,
                        Polynomial_2,
                        Gcd<Polynomial_2>,
                        Pair_cannonicalize<Polynomial_2>,
                        Polynomial_2_compare> Gcd_cache_2;

    //!@}

public:
    //!\name cache access functions
    //!@{
                        
    //! access to the gcd_cache
    Gcd_cache_2& gcd_cache_2() const {
        return *_m_gcd_cache_2;
    }

    //! access to the curve cache
    Curve_cache_2& curve_cache_2() const 
    {
        return *_m_curve_cache_2;
    }
    
    //! access to the curve pair cache
    Curve_pair_cache_2& curve_pair_cache_2() const 
    {
        return *_m_curve_pair_cache_2;
    }
    
    //!@}
    //! \name public functors and predicates
    //!@{
       
                
public:
    //! \brief default constructor 
    Algebraic_curve_kernel_2() 
      : _m_curve_cache_2(new Curve_cache_2(this)), 
        _m_curve_pair_cache_2(new Curve_pair_cache_2(this)),
        _m_gcd_cache_2(new Gcd_cache_2())
    {  
      // std::cout << "CONSTRUCTION  Algebraic_curve_kernel_2 " << std::endl; 
    }
    
public: 
    static Algebraic_curve_kernel_2& get_static_instance(){
      // a default constructed ack_2 instance
      static Algebraic_curve_kernel_2 ack_2_instance;
      return ack_2_instance;
    }

    /*! \brief
     * constructs \c Curve_analysis_2 from bivariate polynomial, uses caching
     * when appropriate
     */
    class Construct_curve_2 :
        public std::unary_function< Polynomial_2, Curve_analysis_2 > {

    public:

        Construct_curve_2(const Algebraic_kernel_2* kernel) : _m_kernel(kernel) {}
            
        Curve_analysis_2 operator()(const Polynomial_2& f, 
                                    Bound angle,
                                    long final_prec) {
            
#if CGAL_ACK_DEBUG_FLAG
            CGAL_ACK_DEBUG_PRINT << "angle=" << angle << std::endl;
            CGAL_ACK_DEBUG_PRINT << "final_prec=" << final_prec << std::endl;
#endif          
            std::pair<Bound,Bound> sin_cos
                = approximate_sin_and_cos_of_angle(angle,final_prec);

            Bound sine = sin_cos.first, cosine = sin_cos.second;            

            
            typedef typename CGAL::Polynomial_traits_d<Polynomial_2>
                ::template Rebind<Bound,1>::Other::Type
                Poly_rat_1;

            typedef typename CGAL::Polynomial_traits_d<Polynomial_2>
                ::template Rebind<Bound,2>::Other::Type
                Poly_rat_2;

            Poly_rat_2 
                sub_x(Poly_rat_1(Bound(0), cosine), Poly_rat_1(sine)), 
                    sub_y(Poly_rat_1(Bound(0), -sine), Poly_rat_1(cosine)), 
                res;
            
            std::vector<Poly_rat_2> subs;
            subs.push_back(sub_x);
            subs.push_back(sub_y);
            
            res = typename CGAL::Polynomial_traits_d<Polynomial_2>
                ::Substitute() (f, subs.begin(), subs.end());

            CGAL::simplify(res);
            
            // integralize polynomial
            typedef CGAL::Fraction_traits<Poly_rat_2> FT;
            typename FT::Denominator_type dummy;
            Polynomial_2 num;
            typename FT::Decompose()(res, num, dummy);

#if CGAL_ACK_DEBUG_FLAG
            CGAL_ACK_DEBUG_PRINT << "integralized poly: " << num << std::endl; 
#endif
            
            return _m_kernel->curve_cache_2()(num);
        }

      Curve_analysis_2 operator()
        (const Polynomial_2& f) const {
        return _m_kernel->curve_cache_2()(f);
      }

    protected:

        const Algebraic_kernel_2* _m_kernel;
        

    };
    CGAL_Algebraic_Kernel_cons(Construct_curve_2, construct_curve_2_object);

    /*! \brief
     * constructs \c Curve_pair_analysis_2 from pair of one curve analyses,
     * caching is used when appropriate
     */
    class Construct_curve_pair_2 :
            public std::binary_function<Curve_analysis_2, Curve_analysis_2,
                Curve_pair_analysis_2> {

    public:

        Construct_curve_pair_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
           
        Curve_pair_analysis_2 operator()
           (const Curve_analysis_2& ca1, const Curve_analysis_2& ca2) const {
                
            Curve_pair_analysis_2 cpa_2 =
                _m_kernel->curve_pair_cache_2()(std::make_pair(ca1, ca2));
            return cpa_2;
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Construct_curve_pair_2,
        construct_curve_pair_2_object);
            
    //! returns the x-coordinate of an \c Xy_coordinate_2 object
    class Get_x_2 :
        public std::unary_function<Xy_coordinate_2, X_coordinate_1> {

    public:

        Get_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        
        X_coordinate_1 operator()(const Xy_coordinate_2& xy) const {
            return xy.x();
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Get_x_2, get_x_2_object);
    
    /*! 
     * \brief returns the y-coordinate of \c Xy_coordinate_2 object
     *
     * \attention{This method returns the y-coordinate in isolating interval
     * representation. Calculating such a representation is usually a time-
     * consuming taks, since it is against the "y-per-x"-view that we take
     * in our kernel. Therefore, it is recommended, if possible,
     *  to use the functors
     * \c Lower_bound_y_2 and \c Upper_bound_y_2 instead that 
     * return approximation of the y-coordinate. The approximation can be
     * made arbitrarily good by iteratively calling \c Refine_y_2.}
     */
    class Get_y_2 :
        public std::unary_function<Xy_coordinate_2, X_coordinate_1> {
        
    public:
        
        Get_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        X_coordinate_1 operator()(const Xy_coordinate_2& xy) const {
            return xy.y();
        }
    protected:
        
        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Get_y_2, get_y_2_object);
    


    class Approximate_absolute_x_2 
    : public std::binary_function<Xy_coordinate_2,int,std::pair<Bound,Bound> >{
    
    public:

        Approximate_absolute_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        std::pair<Bound,Bound> operator() (Xy_coordinate_2 xy,
                                    int prec) {
            Get_x_2 get_x = _m_kernel->get_x_2_object();
            return _m_kernel->approximate_absolute_1_object() 
	      (get_x(xy),prec);
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;
        
    };
    CGAL_Algebraic_Kernel_cons(Approximate_absolute_x_2, 
                               approximate_absolute_x_2_object);

    class Approximate_relative_x_2 
    : public std::binary_function<Xy_coordinate_2,int,std::pair<Bound,Bound> >{
    
    public:
        
        Approximate_relative_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        std::pair<Bound,Bound> operator() (Xy_coordinate_2 xy,
                                           int prec) {
            Get_x_2 get_x = this->get_x_2_object();
            return this->approximate_absolute_x_1_object() (get_x(xy),prec);
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Approximate_relative_x_2, 
                               approximate_relative_x_2_object);

    class Approximate_absolute_y_2 
    : public std::binary_function<Xy_coordinate_2,int,std::pair<Bound,Bound> >{

    public:

        Approximate_absolute_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        
        std::pair<Bound,Bound> operator() (Xy_coordinate_2 xy,
                                    int prec) {
            
            Bound l = xy.lower_bound_y();  
            Bound u = xy.upper_bound_y();
            Bound error = CGAL::ipower(Bound(2),CGAL::abs(prec));
            while((u-l)*error>Bound(1)) {
                xy.refine_y();
                u = xy.upper_bound_y();
                l = xy.lower_bound_y();
          }
          return std::make_pair(l,u);
        }
    
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Approximate_absolute_y_2, 
                               approximate_absolute_y_2_object);

    class Approximate_relative_y_2 
    : public std::binary_function<Xy_coordinate_2,int,std::pair<Bound,Bound> >{
        
    public:
        
        Approximate_relative_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        std::pair<Bound,Bound> operator() (Xy_coordinate_2 xy,
                                           int prec) {
            if(xy.is_y_zero()) {
                return std::make_pair(Bound(0),Bound(0));
            }
            while(CGAL::sign(xy.lower_bound_y())*CGAL::sign(xy.upper_bound_y())
                  !=CGAL::POSITIVE) {
                xy.refine_y();
            }
            Bound l = xy.lower_bound_y();  
            Bound u = xy.upper_bound_y();
            Bound error = CGAL::ipower(Bound(2),CGAL::abs(prec));
            Bound max_b = (CGAL::max)(CGAL::abs(u),CGAL::abs(l));
            while((prec>0)?((u-l)*error>max_b):((u-l)>error*max_b)){
                xy.refine_y();
                u = xy.upper_bound_y();
                l = xy.lower_bound_y();
                max_b = (CGAL::max)(CGAL::abs(u),CGAL::abs(l));
          }
          return std::make_pair(l,u);
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Approximate_relative_y_2, 
                               approximate_relative_y_2_object);

    
    /*! 
     * \brief returns a value of type \c Bound that lies between
     * the x-coordinates of the \c Xy_coordinate_2s.
     *
     * \pre{The x-coordinates must not be equal}
     */
    class Bound_between_x_2 {

    public:
        
        Bound_between_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
       
        typedef Xy_coordinate_2 first_agrument_type;
        typedef Xy_coordinate_2 second_agrument_type;
        typedef Bound result_type;
            
        result_type operator()(const Xy_coordinate_2& r1, 
                const Xy_coordinate_2& r2) const {
 	  return this->_m_kernel->bound_between_1_object()
                (r1.x(), r2.x());
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Bound_between_x_2, 
            bound_between_x_2_object);
            
    /*! 
     * \brief returns a value of type \c Bound that lies between
     * the y-coordinates of the \c Xy_coordinate_2s.
     *
     * \pre{The y-coordinates must not be equal}
     */
    class Bound_between_y_2 {
       
    public:
        
        Bound_between_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        typedef Xy_coordinate_2 first_agrument_type;
        typedef Xy_coordinate_2 second_agrument_type;
        typedef Bound result_type;

	typedef typename Algebraic_kernel_2::Curve_analysis_2
	  ::Status_line_1::Bitstream_descartes Isolator;

        result_type operator()(const Xy_coordinate_2& r1, 
                const Xy_coordinate_2& r2) const {

            CGAL_precondition(r1.y() != r2.y());

            Bound res(0);

            Isolator isol1 =
                r1.curve().status_line_at_exact_x(r1.x()).isolator();

            Isolator isol2 =
                r2.curve().status_line_at_exact_x(r2.x()).isolator();
            
            Bound low1, low2, high1, high2;
            
            while (true) {
                low1 = isol1.left_bound(r1.arcno());
                high1 = isol1.right_bound(r1.arcno());
                
                low2 = isol2.left_bound(r2.arcno());
                high2 = isol2.right_bound(r2.arcno());
                
                if (low1 > high2) {
                    res = ((low1 + high2)/Bound(2));
                    break;
                }
                if (low2 > high1) {
                    res = ((low2 + high1)/Bound(2));
                    break;
                }
                
                // else
                isol1.refine_interval(r1.arcno());
                isol2.refine_interval(r2.arcno());
            }

            CGAL::simplify(res);

            CGAL_postcondition_code(
                    CGAL::Comparison_result exp = CGAL::SMALLER
            );
            CGAL_postcondition_code(
                    if (r1.y() > r2.y()) {
                        exp = CGAL::LARGER;
                    }
            );
            CGAL_postcondition(r1.y().compare(res) == exp);
            CGAL_postcondition(r2.y().compare(res) == -exp);

            return res;
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Bound_between_y_2, 
            bound_between_y_2_object);
    
    //! \brief comparison of x-coordinates 
    class Compare_x_2 :
         public std::binary_function<X_coordinate_1, X_coordinate_1, 
                Comparison_result > {

    public:
        
        Compare_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        Comparison_result operator()(const X_coordinate_1& x1, 
                                     const X_coordinate_1& x2) const {
            return x1.compare(x2);
        }
        Comparison_result operator()(const Xy_coordinate_2& xy1, 
                                     const Xy_coordinate_2& xy2) const {
            return (*this)(xy1.x(), xy2.x());
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_pred(Compare_x_2, compare_x_2_object);

    /*! 
     * \brief comparison of y-coordinates of two points
     *
     * \attention{If both points have different x-coordinates, this method
     * has to translate both y-coordinates 
     * into isolating interval representations which is a time-consuming
     * operation (compare the documentation of the \c Get_y_2 functor)
     * If possible, it is recommended to avoid this functor for efficiency.}
     */
    class Compare_y_2 :
        public std::binary_function< Xy_coordinate_2, Xy_coordinate_2, 
                Comparison_result > {
    
    public:

        Compare_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
    
        Comparison_result operator()(const Xy_coordinate_2& xy1, 
                                     const Xy_coordinate_2& xy2) const {
            
            // It is easier if the x coordinates are equal!
            if(_m_kernel->compare_x_2_object()(xy1.x(), xy2.x()) ==
                    CGAL::EQUAL) 
                return _m_kernel->compare_xy_2_object()(xy1, xy2, true);
            
            return _m_kernel->compare_x_2_object()(xy1.y(), xy2.y());
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;
        
    };
    CGAL_Algebraic_Kernel_pred(Compare_y_2, compare_y_2_object);
    
    /*! 
     * \brief lexicographical comparison of two \c Xy_coordinate_2 objects
     *
     * \param equal_x if set, the points are assumed 
     * to have equal x-coordinates, thus only the y-coordinates are compared.
     */
    class Compare_xy_2 :
          public std::binary_function<Xy_coordinate_2, Xy_coordinate_2, 
                Comparison_result > {

    public:

         Compare_xy_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
    
         Comparison_result operator()(const Xy_coordinate_2& xy1, 
             const Xy_coordinate_2& xy2, bool equal_x = false) const {

             // handle easy cases first
             /*if(xy1.is_identical(xy2))
                return CGAL::EQUAL;
                
             if(equal_x && xy1.curve().is_identical(xy2.curve()))
                return CGAL::sign(xy1.arcno() - xy2.arcno());
                
             bool swap = (xy1.id() > xy2.id());
             std::pair<Xy_coordinate_2, Xy_coordinate_2> p(xy1, xy2);
             if(swap) {
                 p.first = xy2;
                 p.second = xy1;
             }
           
             typename Cmp_xy_map::Find_result r =
                _m_kernel->_m_cmp_xy.find(p);
             if(r.second) {
               //std::cerr << "Xy_coordinate2: precached compare_xy result\n";
                 return (swap ? -(r.first->second) : r.first->second);
             }*/

            return xy1.compare_xy(xy2, equal_x);             
             //_m_kernel->_m_cmp_xy.insert(std::make_pair(p, res));
             //return (swap ? -res : res);
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_pred(Compare_xy_2, compare_xy_2_object);
    
    /*!
     * \brief checks whether the curve induced by \c p 
     * has only finitely many self-intersection points
     *
     * In algebraic terms, it is checked whether  
     * the polynomial \c p is square free.
     */
    class Has_finite_number_of_self_intersections_2 :
            public std::unary_function< Polynomial_2, bool > {
        
    public:

        Has_finite_number_of_self_intersections_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        
        bool operator()(const Polynomial_2& p) const {

            typename Polynomial_traits_2::Is_square_free is_square_free;
            return is_square_free(p);
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;
        
    };
    CGAL_Algebraic_Kernel_pred(Has_finite_number_of_self_intersections_2, 
            has_finite_number_of_self_intersections_2_object);
            
    /*! 
     * \brief checks whether two curves induced bt \c f and \c g 
     * habe finitely many intersections.
     *
     * In algebraic terms, it is checked whether 
     * the two polynomials \c f and \c g are coprime.
     */ 
    class Has_finite_number_of_intersections_2 :
        public std::binary_function< Polynomial_2, Polynomial_2, bool > {
         
    public:

        Has_finite_number_of_intersections_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        bool operator()(const Polynomial_2& f,
                        const Polynomial_2& g) const {
            // if curve ids are the same - non-decomposable
            if(f.id() == g.id())
                return true;
            typename Polynomial_traits_2::Gcd_up_to_constant_factor gcd_utcf;
            typename Polynomial_traits_2::Total_degree total_degree;
             return (total_degree(gcd_utcf(f, g)) == 0);
        }

    protected:
        
        const Algebraic_kernel_2* _m_kernel;
        
    };
  CGAL_Algebraic_Kernel_pred(Has_finite_number_of_intersections_2, 
            has_finite_number_of_intersections_2_object);
    
  // Square_free_factorize_2
  class Square_free_factorize_2 {

  public:
      
      Square_free_factorize_2(const Algebraic_kernel_2* kernel) 
          : _m_kernel(kernel) {}
      
      typedef Polynomial_2 first_argument_type;
      template< class OutputIterator>
      OutputIterator operator()( const Polynomial_2& p, OutputIterator it) 
          const {
          return CGAL::square_free_factorize_up_to_constant_factor(p,it);
      } 
  
  protected:

        const Algebraic_kernel_2* _m_kernel;

  };
  CGAL_Algebraic_Kernel_cons(
          Square_free_factorize_2, square_free_factorize_2_object);

  //this is deprecated ! 
    //! Various curve and curve pair decomposition functions
    class Decompose_2 {
    
    public:

        Decompose_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        //! returns the square free part of the curve induced by \c p
        Polynomial_2 operator()(const Polynomial_2& p) {
            typename Polynomial_traits_2::Make_square_free msf;
            return msf(p);
        }
        
        /*! 
         * \brief computes a square-free factorization of a curve \c c, 
         * returns the number of pairwise coprime square-free factors
         * 
         * returns square-free pairwise coprime factors in \c fit and
         * multiplicities in \c mit. The value type of \c fit is
         * \c Curve_analysis_2, the value type of \c mit is \c int
         */
        template< class OutputIterator1, class OutputIterator2 >
        int operator()(const Curve_analysis_2& ca,
                     OutputIterator1 fit, OutputIterator2 mit ) const {
                        
            typename Polynomial_traits_2::
                Square_free_factorize_up_to_constant_factor factorize;
            std::vector<Polynomial_2> factors;
            
            int n_factors = factorize(ca.polynomial_2(),
                                      std::back_inserter(factors), mit);
            Construct_curve_2 cc_2 = _m_kernel->construct_curve_2_object();
            for(int i = 0; i < static_cast<int>(factors.size()); i++) 
                *fit++ = cc_2(factors[i]);
            
            return n_factors;
        }
        
        /*!\brief
         * Decomposes two curves \c ca1 and \c ca2 into common part
         * and coprime parts
         *
         * The common part of the curves \c ca1 and \c ca2 is written in
         * \c oib, the coprime parts are written to \c oi1 and \c oi2,
         * respectively.
         *         
         * \return {true, if the two curves were not coprime (i.e., have a 
         * non-trivial common part}
         *
         * The value type of \c oi{1,2,b} is \c Curve_analysis_2
         */
        template < class OutputIterator > 
        bool operator()(const Curve_analysis_2& ca1,
            const Curve_analysis_2& ca2, OutputIterator oi1,
                OutputIterator oi2, OutputIterator oib) const {

#if CGAL_ACK_DONT_CHECK_POLYNOMIALS_FOR_COPRIMALITY
        return false;  
#else 

            Construct_curve_2 cc_2 = _m_kernel->construct_curve_2_object();
#if CGAL_ACK_USE_EXACUS
            typedef std::vector<Internal_curve_2> Curves;

            Curves parts_f, parts_g;

            if(Internal_curve_2::decompose(ca1._internal_curve(),
                                           ca2._internal_curve(), 
                                           std::back_inserter(parts_f),
                                           std::back_inserter(parts_g))) {
                typename Curves::const_iterator cit;
                // this is temporary solution while curves are cached on
                // AlciX level
                CGAL_precondition(parts_f[0].polynomial_2() == 
                                  parts_g[0].polynomial_2());
                *oib++ = cc_2(parts_f[0].polynomial_2());
                
                if(parts_f.size() > 1)
                    for(cit = parts_f.begin() + 1; cit != parts_f.end(); cit++)
                        *oi1++ = cc_2(cit->polynomial_2());
                if(parts_g.size() > 1)
                    for(cit = parts_g.begin() + 1; cit != parts_g.end(); cit++)
                        *oi2++ = cc_2(cit->polynomial_2());
                return true;
            }
                
                
#else          

            if (ca1.id() == ca2.id()) {
                return false;
            }

            const Polynomial_2& f = ca1.polynomial_2();
            const Polynomial_2& g = ca2.polynomial_2();
            
            if(f == g) {
              // both curves are equal, but have different representations!
              // std::cout <<"f: " << f <<std::endl;
              // std::cout <<"g: " << g <<std::endl;
              CGAL_assertion(false);
              return false;
            }
            Gcd_cache_2& gcd_cache = _m_kernel->gcd_cache_2();
            typedef typename Curve_analysis_2::size_type size_type;
            Polynomial_2 gcd = gcd_cache(std::make_pair(f,g));
            size_type n = CGAL::degree(gcd);
            size_type nc = CGAL::degree(
                    CGAL::univariate_content_up_to_constant_factor(gcd));
            if( n!=0 || nc!=0 ) {
                Curve_analysis_2 common_curve = cc_2(gcd);
                *oib++ = common_curve;
                Polynomial_2 divided_curve 
                    = CGAL::integral_division(f,gcd);
                if( CGAL::degree(divided_curve)>=1 || 
                    CGAL::degree(
                            CGAL::univariate_content_up_to_constant_factor
                            (divided_curve)) >=1 ) {
                    Curve_analysis_2 divided_c = cc_2(divided_curve);
                    *oi1++ = divided_c;
                }
                divided_curve = CGAL::integral_division(g,gcd);
                if(CGAL::degree(divided_curve) >= 1 ||
                   CGAL::degree(
                           CGAL::univariate_content_up_to_constant_factor
                           ( divided_curve )) >=1 ) {
                    Curve_analysis_2 divided_c = cc_2(divided_curve);
                    *oi2++ = divided_c;
                }
                return true;
            }

#endif
                
            // copy original curves to the output iterator:
            *oi1++ = ca1;
            *oi2++ = ca2;
            return false;
#endif
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Decompose_2, decompose_2_object);
    
    //!@}
public:
    //! \name types and functors for \c CurvedKernelViaAnalysis_2
    //!@{
    
    //! Algebraic name
    typedef X_coordinate_1 Algebraic_real_1;

    //! Algebraic name
    typedef Xy_coordinate_2 Algebraic_real_2;


    //! Algebraic name
    typedef Has_finite_number_of_self_intersections_2 Is_square_free_2;

    //! Algebraic name
    typedef Has_finite_number_of_intersections_2 Is_coprime_2;

    //! Algebraic name
    typedef Decompose_2 Make_square_free_2;

    //! Algebraic name
    typedef Decompose_2 Make_coprime_2;
    
    /*!
     * \brief computes the x-critical points of of a curve/a polynomial
     *
     * An x-critical point (x,y) of \c f (or its induced curve) 
     * satisfies f(x,y) = f_y(x,y) = 0, 
     * where f_y means the derivative w.r.t. y.
     * In pariticular, each singular point is x-critical.
     */
    class X_critical_points_2 : 
        public std::binary_function< Curve_analysis_2, 
            std::iterator<std::output_iterator_tag, Xy_coordinate_2>,
            std::iterator<std::output_iterator_tag, Xy_coordinate_2> > {
       

    public:
        
        X_critical_points_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        /*! 
         * \brief writes the x-critical points of \c ca_2 into \c oi 
         */
        template <class OutputIterator>
        OutputIterator operator()(const Curve_analysis_2& ca_2,
                OutputIterator oi) const {
                
            typename Polynomial_traits_2::Differentiate diff;
            Construct_curve_2 cc_2 = _m_kernel->construct_curve_2_object();
            Construct_curve_pair_2 ccp_2 
                = _m_kernel->construct_curve_pair_2_object();
            // construct curve analysis of a derivative in y
            Curve_analysis_2 ca_2x = cc_2(diff(ca_2.polynomial_2(),0));
            Curve_pair_analysis_2 cpa_2 = ccp_2(ca_2, ca_2x);
            typename Curve_pair_analysis_2::Status_line_1 cpv_line;
            typename Curve_analysis_2::Status_line_1 cv_line;
            
            int i, j, n_arcs, n_events =
                cpa_2.number_of_status_lines_with_event();
            std::pair<int,int> ipair;
            bool vline_constructed = false;
            
            for(i = 0; i < n_events; i++) {
                cpv_line = cpa_2.status_line_at_event(i);
                // no 2-curve intersections over this status line
                if(!cpv_line.is_intersection())
                    continue;
                n_arcs = cpv_line.number_of_events();
                for(j = 0; j < n_arcs; j++) {
                    ipair = cpv_line.curves_at_event(j, ca_2,ca_2x);
                    if(ipair.first == -1|| ipair.second == -1) 
                        continue;
                    if(!vline_constructed) {
                        cv_line = ca_2.status_line_at_exact_x(cpv_line.x());
                        vline_constructed = true;
                    }
                    // ipair.first is an arcno over status line of the
                    // curve p
                    *oi++ = cv_line.algebraic_real_2(ipair.first);
                }
                vline_constructed = false;
            }
            return oi;
        }
        
        //! \brief computes the \c i-th x-critical point of  \c ca
        Xy_coordinate_2 operator()(const Curve_analysis_2& ca, int i) const
        {
            std::vector<Xy_coordinate_2> x_points;
            (*this)(ca, std::back_inserter(x_points));
            CGAL_precondition(0 >= i&&i < x_points.size());
            return x_points[i];
        }
    
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(X_critical_points_2,
        x_critical_points_2_object);
    
    /*!
     * \brief computes the y-critical points of of a curve/a polynomial
     *
     * An y-critical point (x,y) of \c f (or its induced curve) 
     * satisfies f(x,y) = f_x(x,y) = 0, 
     * where f_x means the derivative w.r.t. x.
     * In pariticular, each singular point is y-critical.
     */
    class Y_critical_points_2 :
        public std::binary_function< Curve_analysis_2, 
            std::iterator<std::output_iterator_tag, Xy_coordinate_2>,
            std::iterator<std::output_iterator_tag, Xy_coordinate_2> > {
        

    public:

        Y_critical_points_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        /*! 
         * \brief writes the y-critical points of \c ca_2 into \c oi 
         */
        template <class OutputIterator>
        OutputIterator operator()(const Curve_analysis_2& ca_2, 
            OutputIterator oi) const
        {
            Construct_curve_2 cc_2 = _m_kernel->construct_curve_2_object();
            Construct_curve_pair_2 ccp_2 
                = _m_kernel->construct_curve_pair_2_object();
            
            typename Curve_analysis_2::Status_line_1 cv_line;
            std::pair<int,int> ipair;
            int i, j, k, n_arcs, n_events =
                ca_2.number_of_status_lines_with_event();
            
            bool cpa_constructed = false, vline_constructed = false; 
            typename Curve_pair_analysis_2::Status_line_1
                cpv_line;
            Curve_pair_analysis_2 cpa_2;
            
            for(i = 0; i < n_events; i++) {
                cv_line = ca_2.status_line_at_event(i);
                n_arcs = cv_line.number_of_events();
                for(j = 0; j < n_arcs; j++) {
                    ipair = cv_line.number_of_incident_branches(j);
                    // general case: no special tests required
                    if(!(ipair.first == 1&&ipair.second == 1)) {
                        *oi++ = cv_line.algebraic_real_2(j);
                        continue;
                    }
                    if(!cpa_constructed) {
                        typename Polynomial_traits_2::Differentiate diff;
                        // construct curve analysis of a derivative in y
                        Curve_analysis_2 ca_2y =
                            cc_2(diff(ca_2.polynomial_2(),1));
                        cpa_2 = ccp_2(ca_2, ca_2y);
                        cpa_constructed = true;
                    }
                    if(!vline_constructed) {
                        cpv_line = cpa_2.status_line_for_x(cv_line.x());
                        vline_constructed = true;
                    }
                    if(!cpv_line.is_intersection())
                        continue;
                    // obtain the y-position of j-th event of curve p
                    k = cpv_line.event_of_curve(j, ca_2);
                    ipair = cpv_line.curves_at_event(k);
                    
                    // pick up only event comprised of both curve and its der
                    if(ipair.first != -1&&ipair.second != -1)
                        *oi++ = cv_line.algebraic_real_2(j);
                }
                vline_constructed = false;
            }
            return oi;
        }

        //! \brief computes the \c i-th x-critical point of  \c ca
        Xy_coordinate_2 operator()(const Curve_analysis_2& ca, int i) const
        {
            std::vector<Xy_coordinate_2> y_points;
            (*this)(ca, std::back_inserter(y_points));
            CGAL_precondition(0 >= i&&i < y_points.size());
            return y_points[i];
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Y_critical_points_2,
        y_critical_points_2_object);

    /*!
     * \brief sign computation of a point and a curve
     *
     * computes the sign of a point \c p, evaluate at the polynomial
     * that defines a curve \c c. If the result is 0, the point lies on the
     * curve. Returns a value convertible to \c CGAL::Sign
     */
    class Sign_at_2 :
        public std::binary_function<Curve_analysis_2, Xy_coordinate_2, Sign > {

    public:
        
        Sign_at_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
       
        //! type for bound intervals
        typedef boost::numeric::interval<Bound> Bound_interval;
        typedef CGAL::Coercion_traits<Coefficient,Bound> Coercion;
        typedef typename CGAL::Coercion_traits<Coefficient,Bound>::Type
	  Coercion_type;
        typedef boost::numeric::interval<Coercion_type> Coercion_interval;

	/*        
        typedef typename CGAL::Polynomial_traits_d<Polynomial_2>
            ::template Rebind<Bound,1>::Other::Type
            Poly_rat_1;
        typedef typename CGAL::Polynomial_traits_d<Polynomial_2>
            ::template Rebind<Bound,2>::Other::Type
            Poly_rat_2;
	*/        

        Sign operator()(const Polynomial_2& f,
                        const Xy_coordinate_2& r) const {

            return (*this)(_m_kernel->construct_curve_2_object()(f),r);
        }

        Sign operator()(const Curve_analysis_2& ca_2,
                        const Xy_coordinate_2& r) const {
                
            if(ca_2.is_identical(r.curve()) || _test_exact_zero(ca_2, r))
                return CGAL::ZERO;
            
	    typename Algebraic_kernel_2::Approximate_absolute_x_2 approx_x
	      = _m_kernel->approximate_absolute_x_2_object();
	    typename Algebraic_kernel_2::Approximate_absolute_y_2 approx_y
	      = _m_kernel->approximate_absolute_y_2_object();
	    
	    long prec = 16;

            while(true) {
  	        std::pair<Bound,Bound> x_pair = approx_x(r,prec);
  	        std::pair<Bound,Bound> y_pair = approx_y(r,prec);
		
                Coercion_interval iv
                  = interval_evaluate_2(ca_2.polynomial_2(),x_pair,y_pair);
                CGAL::Sign s_lower = CGAL::sign(iv.lower());
                if(s_lower == sign(iv.upper()))
                    return s_lower;
                prec*=2;
	    }
        }
        
    protected:

        template<typename Polynomial_2>
          Coercion_interval interval_evaluate_2(const Polynomial_2& p,
						std::pair<Bound,Bound> x_pair,
						std::pair<Bound,Bound> y_pair)
	  const {
        
            typename Coercion::Cast cast;

            typedef typename CGAL::Polynomial_traits_d<Polynomial_2>::
              Coefficient_const_iterator
              Coefficient_const_iterator;
        
            typedef typename CGAL::Polynomial_traits_d<Polynomial_2>
              ::Coefficient_const_iterator_range 
              Coefficient_const_iterator_range;
        
            Coercion_interval iy(cast(y_pair.first),
                                 cast(y_pair.second));

            // CGAL::Polynomial does not provide Coercion_traits for number
            // types => therefore evaluate manually
            Coefficient_const_iterator_range range =
              typename CGAL::Polynomial_traits_d<Polynomial_2>
              :: Construct_coefficient_const_iterator_range()(p);
        
            Coefficient_const_iterator it = range.second - 1;
        
            Coercion_interval res(interval_evaluate_1(*it,x_pair));
        
            Coefficient_const_iterator p_begin = range.first;

            while((it--) != p_begin) 
	      res = res * iy + (interval_evaluate_1(*it,x_pair));
            return res;
	}
	
    
        template<typename Polynomial_1>
          Coercion_interval interval_evaluate_1(const Polynomial_1& p,
		  			        std::pair<Bound,Bound> x_pair)
	  const {
    
            typename Coercion::Cast cast;
     
            typedef typename CGAL::Polynomial_traits_d<Polynomial_1>
              ::Coefficient_const_iterator Coefficient_const_iterator;

            Coercion_interval ix(cast(x_pair.first),
                                 cast(x_pair.second));
	    
	    typedef typename CGAL::Polynomial_traits_d<Polynomial_1>
              ::Coefficient_const_iterator_range 
              Coefficient_const_iterator_range;
        
            Coefficient_const_iterator_range range = 
              typename CGAL::Polynomial_traits_d<Polynomial_1>
              :: Construct_coefficient_const_iterator_range()(p);
        
            Coefficient_const_iterator it = range.second - 1;
        
            Coercion_interval res(cast(*it));

            Coefficient_const_iterator p_begin = range.first;
            while((it--) != p_begin) 
              res = res * ix + Coercion_interval(cast(*it));
            return res;
	}

        bool _test_exact_zero(const Curve_analysis_2& ca_2,
            const Xy_coordinate_2& r) const {

            Polynomial_2 zero_p(Coefficient(0));
            if (ca_2.polynomial_2() == zero_p) {
                return true;
            }

            Construct_curve_2 cc_2 = _m_kernel->construct_curve_2_object();
            Construct_curve_pair_2 ccp_2 
                = _m_kernel->construct_curve_pair_2_object();
            typename Curve_analysis_2::Status_line_1
                cv_line = ca_2.status_line_for_x(r.x());
            // fast check for the presence of status line at r.x()
            if(cv_line.covers_line())    
                return true;

            // Handle non-coprime polynomial
            Polynomial_2 gcd = _m_kernel->gcd_cache_2()
               (std::make_pair(ca_2.polynomial_2(), r.curve().polynomial_2()));

            Curve_analysis_2 gcd_curve = cc_2(gcd);
            if(CGAL::total_degree(gcd)>0) {
                
                Construct_curve_pair_2 ccp_2
                    = _m_kernel->construct_curve_pair_2_object();
                Curve_analysis_2 r_curve_remainder =
                    cc_2(CGAL::integral_division_up_to_constant_factor(
                                 r.curve().polynomial_2(), gcd
                         )
                    );
                    
                r.simplify_by(ccp_2(gcd_curve, r_curve_remainder));
                if(r.curve().polynomial_2() == gcd) 
                    return true;
            }

            Curve_pair_analysis_2 cpa_2 = ccp_2(ca_2, r.curve());
            typename Curve_pair_analysis_2::Status_line_1
                cpv_line = cpa_2.status_line_for_x(r.x());
            
            if(cpv_line.is_event() && cpv_line.is_intersection()) {
                // get an y-position of the point r
                int idx = cpv_line.event_of_curve(r.arcno(), r.curve());
                std::pair<int, int> ipair =
                      cpv_line.curves_at_event(idx);
                if(ipair.first != -1 && ipair.second != -1)
                    return true;
            }
            return false;
        }
    protected:
        
        const Algebraic_kernel_2* _m_kernel;

    
    };
    CGAL_Algebraic_Kernel_pred(Sign_at_2, sign_at_2_object);

    /*!
     * \brief computes solutions of systems of two 2 equations and 2 variables
     *
     * \pre the polynomials must be square-free and coprime
     */  
    class Solve_2 {
    
    public:

        Solve_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        typedef Curve_analysis_2 first_argument_type;
        typedef Curve_analysis_2 second_argument_type;
        typedef std::iterator<std::output_iterator_tag, Xy_coordinate_2>
            third_argument_type;
        typedef std::iterator<std::output_iterator_tag, int>
            fourth_argument_type;
        typedef std::pair<third_argument_type, fourth_argument_type>
            result_type;
     
        /*! 
         * \brief solves the system (f=0,g=0)
         *
         * All solutions of the system are written into \c roots 
         * (whose value type is \c Xy_coordinate_2). The multiplicities
         * are written into \c mults (whose value type is \c int)
         */
        template <class OutputIteratorRoots, class OutputIteratorMult>
        std::pair<OutputIteratorRoots, OutputIteratorMult>
           operator()
               (const Polynomial_2& f, const Polynomial_2& g,
                OutputIteratorRoots roots, OutputIteratorMult mults) const {
            return 
                (*this)(_m_kernel->construct_curve_2_object()(f),
                        _m_kernel->construct_curve_2_object()(g),
                        roots,mults);
        }

        //! Version with curve analyses
        template <class OutputIteratorRoots, class OutputIteratorMult>
        std::pair<OutputIteratorRoots, OutputIteratorMult>
           operator()(const Curve_analysis_2& ca1, const Curve_analysis_2& ca2,
                OutputIteratorRoots roots, OutputIteratorMult mults) const
        {
            // these tests are quite expensive... do we really need them ??
            /*
            CGAL_precondition_code (
                typename Self::Has_finite_number_of_self_intersections_2 
                    not_self_overlapped;
                typename Self::Has_finite_number_of_intersections_2 
                    do_not_overlap;
                CGAL_precondition(not_self_overlapped(ca1) &&
                    not_self_overlapped(ca2));
                CGAL_precondition(do_not_overlap(ca1, ca2));
            );
            */
            Construct_curve_pair_2 ccp_2
                = _m_kernel->construct_curve_pair_2_object();
            Curve_pair_analysis_2 cpa_2 = ccp_2(ca1, ca2);
            typename Curve_pair_analysis_2::Status_line_1 cpv_line;
            // do we need to check which supporting curve is simpler ?    
            typename Polynomial_traits_2::Total_degree total_degree;

            Polynomial_2 f1 = ca1.polynomial_2(),
                f2 = ca2.polynomial_2();
            bool first_curve = (total_degree(f1) < total_degree(f2));
            
            int i, j, n = cpa_2.number_of_status_lines_with_event();
            std::pair<int, int> ipair;
            for(i = 0; i < n; i++) {
                cpv_line = cpa_2.status_line_at_event(i);
                X_coordinate_1 x = cpv_line.x(); 
                bool ca1_covers_line 
                    = ca1.status_line_at_exact_x(x).covers_line();
                bool ca2_covers_line 
                    = ca2.status_line_at_exact_x(x).covers_line();
                
                for(j = 0; j < cpv_line.number_of_events(); j++) {
                    ipair = cpv_line.curves_at_event(j,ca1,ca2);
                    if(ipair.first != -1 && ipair.second != -1) {
                        *roots++ 
			  = Xy_coordinate_2(_m_kernel,x, 
                                              (first_curve ? ca1 : ca2),
                                              (first_curve ? ipair.first
                                                           : ipair.second));
                        *mults++ = cpv_line.multiplicity_of_intersection(j);
                        continue;
                    }
                    if(ipair.first!=-1 && ca2_covers_line) {
                        *roots++ 
			  = Xy_coordinate_2(_m_kernel,x,ca1,ipair.first);
                        *mults++ = -1;
                        continue;
                    }
                    if(ipair.second!=-1 && ca1_covers_line) {
                        *roots++ 
			  = Xy_coordinate_2(_m_kernel,x,ca2,ipair.second);
                        *mults++ = -1;
                        continue;
                    }
                }
            }
            return std::make_pair(roots, mults);
        }

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Solve_2, solve_2_object);

    /*!
     * \brief Construct a curve with the roles of x and y interchanged.
     */
    class Swap_x_and_y_2 {

    public:

        Swap_x_and_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        
        typedef Polynomial_2 argument_type;
        typedef Curve_analysis_2 result_type;

        Curve_analysis_2 operator() (const Curve_analysis_2& ca) {
            return this->operator() (ca.polynomial_2());
        }

        Curve_analysis_2 operator() (const Polynomial_2& f) {
            Polynomial_2 f_yx
                = typename Polynomial_traits_2::Swap() (f,0,1);
            return _m_kernel->construct_curve_2_object() (f_yx);
        }
    
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Swap_x_and_y_2, swap_x_and_y_2_object);

#if CGAL_AK_ENABLE_DEPRECATED_INTERFACE

    //! Refines the x-coordinate of an Xy_coordinate_2 object
    class Refine_x_2 :
        public std::unary_function<Xy_coordinate_2, void> {

    public:
        
        Refine_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
      
        void operator()(const Xy_coordinate_2& r) const {
            r.refine_x();            
        }
	/* TODO: if needed, include
        void operator()(Xy_coordinate_2& r, int rel_prec) const {  
            r.refine_x(rel_prec);
        }
	*/
        
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_pred(Refine_x_2, refine_x_2_object);
    
    class Refine_y_2 :
        public std::unary_function<Xy_coordinate_2, void> {

    public:

        Refine_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
      
        void operator()(const Xy_coordinate_2& r) const {
  	    return r.refine_y();
        }
        
	/* TODO: if needed, include
        void operator()(Xy_coordinate_2& r, int rel_prec) const {  
            return r.refine_y(rel_prec);
        }
	*/
    
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_pred(Refine_y_2, refine_y_2_object);
    
    class Lower_bound_x_2 {
       
    public:

        Lower_bound_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        typedef Xy_coordinate_2 argument_type;
        typedef Bound result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
	    return r.lower_bound_x();
	}
        
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Lower_bound_x_2, lower_bound_x_2_object);
    
    class Upper_bound_x_2 {
       
    public:

        Upper_bound_x_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}

        typedef Xy_coordinate_2 agrument_type;
        typedef Bound result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
            return r.upper_bound_x();
        }
    
    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Upper_bound_x_2, upper_bound_x_2_object);

    class Lower_bound_y_2 {
    
    public:

        Lower_bound_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
        
        typedef Xy_coordinate_2 agrument_type;
        typedef Bound result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
	  return r.lower_bound_y();
	}

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Lower_bound_y_2, lower_bound_y_2_object);
    
    //! an upper bound of the y-coordinate of \c r
    class Upper_bound_y_2 {
    
    public:

        Upper_bound_y_2(const Algebraic_kernel_2* kernel) 
            : _m_kernel(kernel) {}
   
        typedef Xy_coordinate_2 agrument_type;
        typedef Bound result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
 	  return r.upper_bound_y();
	}

    protected:

        const Algebraic_kernel_2* _m_kernel;

    };
    CGAL_Algebraic_Kernel_cons(Upper_bound_y_2, upper_bound_y_2_object);
    


  typedef Bound Boundary; 
  typedef Lower_bound_x_2 Lower_boundary_x_2;
  typedef Lower_bound_y_2 Lower_boundary_y_2;
  typedef Upper_bound_x_2 Upper_boundary_x_2;
  typedef Upper_bound_y_2 Upper_boundary_y_2;
  typedef Bound_between_x_2 Boundary_between_x_2;
  typedef Bound_between_y_2 Boundary_between_y_2;

  CGAL_Algebraic_Kernel_cons(Lower_boundary_x_2,lower_boundary_x_2_object);
  CGAL_Algebraic_Kernel_cons(Lower_boundary_y_2,lower_boundary_y_2_object);
  CGAL_Algebraic_Kernel_cons(Upper_boundary_x_2,upper_boundary_x_2_object);
  CGAL_Algebraic_Kernel_cons(Upper_boundary_y_2,upper_boundary_y_2_object);
  CGAL_Algebraic_Kernel_cons(Boundary_between_x_2,boundary_between_x_2_object);
  CGAL_Algebraic_Kernel_cons(Boundary_between_y_2,boundary_between_y_2_object);
#endif


#undef CGAL_Algebraic_Kernel_pred    
#undef CGAL_Algebraic_Kernel_cons 
    
    //!@}

protected:

mutable boost::shared_ptr<Curve_cache_2> _m_curve_cache_2;
mutable boost::shared_ptr<Curve_pair_cache_2> _m_curve_pair_cache_2;
mutable boost::shared_ptr<Gcd_cache_2> _m_gcd_cache_2;

    
}; // class Algebraic_curve_kernel_2

} // namespace CGAL

#endif // CGAL_ALGEBRAIC_CURVE_KERNEL_2_H
