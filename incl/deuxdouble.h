#pragma once


/**
 * @brief Double-double, to extend precision of double
 */
struct deuxDouble {
    double val = 0.;
    double err = 0.; ///< The error term of the double-double

    deuxDouble() {}
    deuxDouble( double d_) : val(d_), err(0.) {}
};

/**
 * @brief Function to add two Double2 variables
 * 
 * @param dd0_ First double-double variable
 * @param dd1_ Second double-double variable
 * 
 * @return The sum, (dd0_ + dd1_)
 */
deuxDouble dd_add(const deuxDouble& dd0_, const deuxDouble& dd1_) 
{
    //-- TWO-SUM ( dd0_.val, dd1_.val) [Knuth]
    double _x = dd0_.val + dd1_.val;

    // Note: the effective _ds1.val that is added to dd0_.val to give _x;
    double _dd1_val_virtual = _x - dd0_.val;

    // Note: (_x - _dd1_val_virtual) = _dd0_val_virtual, the effective _ds0.val that contributing to _x;
    // Note: _y = all the round off errors in _x;
    double _y = ((dd1_.val - _dd1_val_virtual) + (dd0_.val - (_x - _dd1_val_virtual)));

    // Note: also add existing errors from dd0_ and dd1_
    _y = _y + dd0_.err + dd1_.err;

    //--- FAST-TWO-SUM ( _x, _y) [Dekker], |_x| > |_y|
    deuxDouble _dd;
    _dd.val = _x + _y;
    _dd.err = _y - (_dd.val - _x);

    return _dd;
}

/**
 * @brief Function to subtract two Double2 variables
 *
 * @param dd0_ First double-double variable
 * @param dd1_ Second double-double variable
 * 
 * @return The difference, (dd0_ - dd1_)
 */
deuxDouble dd_sub(const deuxDouble& dd0_, const deuxDouble& dd1_)
{
    double _x = dd0_.val - dd1_.val;
    double _dd1_val_virtual = _x - dd0_.val;

    double _y = ((-dd1_.val - _dd1_val_virtual) + (dd0_.val - (_x - _dd1_val_virtual))) + dd0_.err - dd1_.err;

    deuxDouble _dd;
    _dd.val = _x + _y;
    _dd.err = _y - (_dd.val - _x);

    return _dd;

}






