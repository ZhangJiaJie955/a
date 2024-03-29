#if !defined(AFX_LINE_H__E3A0AA20_F151_11D5_B96A_CCFB30288840__INCLUDED_)
#define AFX_LINE_H__E3A0AA20_F151_11D5_B96A_CCFB30288840__INCLUDED_

#include <list>
#include <opencv2/opencv.hpp>


#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

class C_Line  
{
public:
	//ture if the line doesnot have too large slope and not large ordonnee a l'origine
	inline	bool is_valid() const
	{
		return (fabsf(a)<FLT_MAX && fabsf(b)<FLT_MAX);
	};
	//true if the line is (nearly) a vertical one
	inline	bool is_vertical() const
	{
		return fabsf(a)>FLT_MAX*0.5f;
	};
	void reset();

	//determine si les deux droites sont paralleles
	inline bool is_parallel(const C_Line & line_) const
	{
		const float MAX_FLT_ERROR=0.000001f;
		return (fabsf(a-line_.a)<MAX_FLT_ERROR);
	};
	//retourne l'image de abscisse par la fonction affi,e definie par la droite
	int get_image_entiere(const int abscisse) const;
	//retourne l'image de abscisse par la fonction affi,e definie par la droite
	float get_image(const int abscisse) const;
	//retourne l'image de abscisse par la fonction affi,e definie par la droite
	float get_image(const float& abscisse) const;
	//retourne le coefficient directeur
	inline	float get_skew() const{
		return a;
	};
	//retour l'angle de la droite par rapport a l'horizontale
	inline	float get_skew_angle() const{
		return float(atanf(a));
	};
	//retour l'angle de la droite par rapport a l'horizontale
	inline	float get_skew_angle(const C_Line & l) const{
		return (l.get_skew_angle()- atanf(a));
	};
	
	inline	float get_slant_angle() const{
		const float pi_sur_2=1.570796327f;
		float slant_angle(atanf(a));
#ifdef _DEBUG
		assert(slant_angle > -pi_sur_2 - .1 && slant_angle < pi_sur_2 + .1);
#endif //_DEBUG
		if(slant_angle>0) {

			slant_angle-=pi_sur_2;
#ifdef _DEBUG
			assert(-slant_angle>-FLT_EPSILON);
#endif //_DEBUG
		}
		else {

			slant_angle+=pi_sur_2;									 									 
#ifdef _DEBUG
			assert(-slant_angle<FLT_EPSILON);
#endif //_DEBUG
		}
#ifdef _DEBUG
		assert(slant_angle > -pi_sur_2 - .1 && slant_angle < pi_sur_2 + .1);
#endif //_DEBUG
		return -slant_angle;
	};
	inline	bool is_x_axis() const{
		if(fabsf(a)>FLT_EPSILON) return false;
		else return(fabsf(b)<FLT_EPSILON);
	};
	//l'ordonnee  l'origine
	float b;
	//le coefficient directeur
	float a;

	C_Line();

	C_Line(const C_Line & line_);
	C_Line(const float & a_,const float & b_);
	C_Line(const cv::Point2f& A, const cv::Point2f& B);
	virtual ~C_Line();
	//retourne le pt d'inter de C_Line avec la dte horiz y=ordonnee
	float get_abs(const int ordonnee) const;
	float get_abs(const float ordonnee) const;
	bool is_horizontale() const
	{
		return(fabsf(a)<FLT_EPSILON);
	};
	bool is_nearly_the_same(const C_Line & right_op) const;
protected:
	bool is_nearly_horiz() const;
};

#endif // !defined(AFX_LINE_H__E3A0AA20_F151_11D5_B96A_CCFB30288840__INCLUDED_)

