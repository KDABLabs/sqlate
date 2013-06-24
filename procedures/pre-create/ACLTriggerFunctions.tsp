-- Functions and triggers to restrict access to tables, columns, rows, etc.

-- Function to check if the current user is member of sqladmins group
CREATE OR REPLACE FUNCTION is_administrator()
    RETURNS trigger AS $BODY$
    DECLARE
        var_rolname varchar(50);
    BEGIN
        SELECT ROLNAME INTO var_rolname FROM PG_USER JOIN PG_AUTH_MEMBERS ON (PG_USER.USESYSID=PG_AUTH_MEMBERS.MEMBER) JOIN PG_ROLES ON (PG_ROLES.OID=PG_AUTH_MEMBERS.ROLEID AND PG_ROLES.rolname='sqladmins') WHERE PG_USER.USENAME=current_user;
        IF FOUND THEN
            RETURN NEW;
        ELSE
            RAISE EXCEPTION 'user is not in sqladmins group';
        END IF;
    END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION is_administrator() OWNER TO postgres;
